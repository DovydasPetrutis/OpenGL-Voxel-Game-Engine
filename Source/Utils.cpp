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

uint64_t Utils::xyz_to_hilbert3d(uint32_t x, uint32_t y, uint32_t z, int order)
{
    uint64_t index = 0;

    for (int s = order - 1; s >= 0; --s) {
        uint32_t rx = (x >> s) & 1;
        uint32_t ry = (y >> s) & 1;
        uint32_t rz = (z >> s) & 1;

        uint32_t octant = (rx << 2) | ((rx ^ ry) << 1) | (rx ^ ry ^ rz);

        index = (index << 3) | octant;

        if (rz == 0) {
            if (ry == 0) {
                if (rx == 1) {
                    x = ~x;
                    y = ~y;
                }
            }
            else {
                uint32_t t = x; x = y; y = t;
            }
        }
        else {
            uint32_t t = x; x = ~z; z = ~t;
        }
    }
    return index;
}

glm::ivec3 Utils::hilbert3d_to_xyz(uint64_t index, int order)
{
    uint32_t x = 0, y = 0, z = 0;

    for (int s = order - 1; s >= 0; --s) {
        uint32_t octant = static_cast<uint32_t>((index >> (s * 3)) & 7);

        uint32_t rx = (octant >> 2) & 1;
        uint32_t ry = ((octant >> 1) ^ rx) & 1;
        uint32_t rz = (octant ^ (octant >> 1)) & 1;

        x |= (rx << s);
        y |= (ry << s);
        z |= (rz << s);

        if (rz == 0) {
            if (ry == 0) {
                if (rx == 1) {
                    x = ~x;
                    y = ~y;
                }
            }
            else {
                uint32_t t = x; x = y; y = t;
            }
        }
        else {
            uint32_t t = x; x = ~z; z = ~t;
        }
    }

    uint32_t world_offset = 1U << (order - 1);
    glm::ivec3 chunk_pos;
    chunk_pos.x = static_cast<int32_t>(x) - static_cast<int32_t>(world_offset);
    chunk_pos.y = static_cast<int32_t>(y) - static_cast<int32_t>(world_offset);
    chunk_pos.z = static_cast<int32_t>(z) - static_cast<int32_t>(world_offset);
    return chunk_pos;
}
