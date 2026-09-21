#include "Utils.h"

float Utils::Halton(int index, int base)
{
    float result = 0.0f;
    float f = 1.0f / (float)base;
    float i = (float)index;
    while (i > 0) {
        result += f * (float)fmod(i, (float)base);
        i = floor(i / (float)base);
        f = f / (float)base;
    }
    return result;
}

void Utils::generateOutlineMatrices(glm::mat4& input, std::vector<glm::mat4>& output)
{
    output = std::vector<glm::mat4>{
            input,
            glm::rotate(input,glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::rotate(input,glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::rotate(input,glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),

            glm::rotate(input,glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(input,glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::rotate(input,glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),glm::radians(-90.0f),glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::rotate(input,glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),glm::radians(90.0f),glm::vec3(1.0f, 0.0f, 0.0f)),

            glm::rotate(input,glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::rotate(input,glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::rotate(glm::rotate(input, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),glm::radians(-90.0f),glm::vec3(0.0f,0.0f,1.0f)),
            glm::rotate(glm::rotate(input, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),glm::radians(-180.0f),glm::vec3(0.0f,0.0f,1.0f)),
    };
}


void Utils::change_frustrum(ComputeShader& cullProgram, Shader& projProgram,Camera &camera)
{
    float frustrumY = glm::tan(glm::radians(camera.getFov() / 2.0f));
    float frustrumX = frustrumY * ((float)Settings::width / (float)Settings::height);
    cullProgram.use();
    cullProgram.uniform1f("frustrumX", frustrumX);
    cullProgram.uniform1f("frustrumY", frustrumY);
    cullProgram.uniform1f("cosx", glm::inversesqrt(1 + frustrumX * frustrumX));
    cullProgram.uniform1f("cosy", glm::inversesqrt(1 + frustrumY * frustrumY));
    projProgram.use();
    projProgram.uniformMatrix4fv("proj", glm::perspectiveZO(glm::radians(camera.getFov()), (float)Settings::width / (float)Settings::height, Settings::far_plane, Settings::near_plane));
}


void Utils::strafeMoveCheck(GLFWwindow* window, GLenum sec1, GLenum sec2,const glm::vec3 &direc, float deltaTime,Camera &camera)
{
    if (glfwGetKey(window, sec1) == GLFW_PRESS || glfwGetKey(window, sec2) == GLFW_PRESS)
    {
        camera.cameraPos += direc * deltaTime * camera.cameraSpeed / 1.4142135623730950488016887242097f; // sqrt(2)
    }
    else
    {
        camera.cameraPos += direc * deltaTime * camera.cameraSpeed;
    }
}

uint16_t Utils::MortonEncode16t(uint16_t x, uint16_t y, uint16_t z)
{
    // Inner lambda to spread the first 4 bits of a number so they are separated by two spaces
    // Input:  0000 0000 0000 abcd
    // Output: 0000 0a00 b00c 000d
    auto spreadBits = [](uint16_t n) {
        n &= 0x000F;                 // Force keep only the first 4 bits (just in case)
        n = (n | (n << 8)) & 0x0303; // ---- --32 ---- ---- ---- ---- 10 -> ---- --32 ---- ---- 32-- --10
        n = (n | (n << 4)) & 0x0924; // ---- --32 ---- 32-- --10 -> ---- -3-- 2--1 --0 (Binary: 0000 1001 0010 0100)
        return n;
        };

    // Spread them out, shift y and z into their interleaved slots, and merge
    return spreadBits(x) | (spreadBits(y) << 1) | (spreadBits(z) << 2);
}



uint32_t Utils::xyz_to_Morton32t(uint16_t x, uint16_t y, uint16_t z)
{
    auto splitBy2 = [](uint32_t a) -> uint64_t {
        uint64_t v = a & 0x1FFFFFULL; // Clamp to 21 bits
        v = (v | (v << 32)) & 0x001F00000000FFFFULL;
        v = (v | (v << 16)) & 0x001F0000FF0000FFULL;
        v = (v | (v << 8)) & 0x010F00F00F00F00FULL;
        v = (v | (v << 4)) & 0x10C30C30C30C30C3ULL;
        v = (v | (v << 2)) & 0x1249249249249249ULL;
        return v;
        };

    return splitBy2(x) | (splitBy2(y) << 1) | (splitBy2(z) << 2);
}

glm::ivec3 Utils::Morton32t_to_xyz(uint32_t m)
{
    auto compactBy2 = [](uint64_t v) -> uint32_t {
        v &= 0x1249249249249249ULL;
        v = (v | (v >> 2)) & 0x10C30C30C30C30C3ULL;
        v = (v | (v >> 4)) & 0x010F00F00F00F00FULL;
        v = (v | (v >> 8)) & 0x001F0000FF0000FFULL;
        v = (v | (v >> 16)) & 0x001F00000000FFFFULL;
        v = (v | (v >> 32)) & 0x00000000001FFFFFULL;
        return static_cast<uint32_t>(v);
        };

    return {
        compactBy2(m),
        compactBy2(m >> 1),
        compactBy2(m >> 2)
    };
}

uint32_t Utils::calculate_sphere_block_count(int32_t radius)
{
    // This may be inefficient, but is straightforward
    int32_t maxDistanceSquared = radius * radius;
    int32_t count = 0;
    for (int16_t x = -radius; x <= radius;x++)
    {
        for (int16_t y = -radius; y <= radius;y++)
        {
            for (int16_t z = -radius; z <= radius;z++)
            {
                int32_t distanceSquared = x * x + y * y + z * z;
                if (distanceSquared <= maxDistanceSquared)
                {
                    count++;
                }
            }
        }
    }
    return count;
}
