#include "World.h"
void World::init()
{

    for (int i = -Settings::CHUNK_COUNT_Z / 2; i < Settings::CHUNK_COUNT_Z / 2; i++)
    {
        for (int j = -Settings::CHUNK_COUNT_Y / 2; j < Settings::CHUNK_COUNT_Y / 2; j++)
        {
            for (int k = -Settings::CHUNK_COUNT_X / 2; k < Settings::CHUNK_COUNT_X / 2; k++)
            {
                chunks[Utils::xyz_to_hilbert3d(k, j, i, Settings::CHUNK_ORDER)] = Chunk(k);
            }
        }
    }

    chunk_face_culling(chunks, faceCoordsandData);

    chunkComputeData.resize(chunks.size());

    int computeDataIndex = 0;
    for (int i = -Settings::CHUNK_COUNT_Z / 2; i < Settings::CHUNK_COUNT_Z / 2; i++)
    {
        for (int j = -Settings::CHUNK_COUNT_Y / 2; j < Settings::CHUNK_COUNT_Y / 2; j++)
        {
            for (int k = -Settings::CHUNK_COUNT_X / 2; k < Settings::CHUNK_COUNT_X / 2; k++)
            {
                const Chunk& chunk = chunks.at(Utils::xyz_to_hilbert3d(k, j, i, Settings::CHUNK_ORDER));
                chunkComputeData[computeDataIndex].worldPos = glm::vec4(k * 16.0f, j * 16.0f, i * 16.0f, 1.0f);
                chunkComputeData[computeDataIndex].faceCount = chunk.faceCount;
                chunkComputeData[computeDataIndex].faceOffset = chunk.faceOffset;
                computeDataIndex++;
            }
        }
    }

}

void World::chunk_face_culling(std::unordered_map<uint64_t, Chunk>& chunks, std::vector<uint32_t>& faceCoordsandData)
{
    int32_t chunkCount = chunks.size();
    const int32_t x = Settings::CHUNK_COUNT_X;
    const int32_t y = Settings::CHUNK_COUNT_Y;
    const int32_t z = Settings::CHUNK_COUNT_Z;
    auto index = [](int cx, int cy, int cz) { return Utils::xyz_to_hilbert3d(cx, cy, cz, Settings::CHUNK_ORDER);};

    // Each chunk gets its own face buffer — no shared writes
    const size_t hilbertSize = size_t(1) << (3 * Settings::CHUNK_ORDER);
    std::vector<std::vector<uint32_t>> localFaces(hilbertSize);

    // --- PARALLEL: generate faces per chunk ---
#pragma omp parallel for collapse(3) schedule(dynamic)
    for (int i = -z / 2; i < z / 2; i++)
    {
        for (int j = -y / 2; j < y / 2; j++)
        {
            for (int k = -x / 2; k < x / 2; k++)
            {
                auto chunkIndex = index(k, j, i);
                Chunk* chunk = &chunks.at(chunkIndex);

                Chunk* front = (i > -z / 2) ? &chunks.at(index(k, j, i - 1)) : nullptr;
                Chunk* back = (i < z / 2 - 1) ? &chunks.at(index(k, j, i + 1)) : nullptr;
                Chunk* right = (k < x / 2 - 1) ? &chunks.at(index(k + 1, j, i)) : nullptr;
                Chunk* left = (k > -x / 2) ? &chunks.at(index(k - 1, j, i)) : nullptr;
                Chunk* top = (j < y / 2 - 1) ? &chunks.at(index(k, j + 1, i)) : nullptr;
                Chunk* bottom = (j > -y / 2) ? &chunks.at(index(k, j - 1, i)) : nullptr;

                // Writes only to localFaces[index] — thread-safe
                generateFaces(*chunk, localFaces[index(k, j, i)], front, back, right, left, top, bottom, chunkIndex);
            }
        }
    }

    // --- SEQUENTIAL: merge + compute cumulative faceOffset ---
    int32_t faceOffset = 0;
    for (int i = -z / 2; i < z / 2; i++)
    {
        for (int j = -y / 2; j < y / 2; j++)
        {
            for (int k = -x / 2; k < x / 2; k++)
            {
                uint64_t idx = index(k, j, i);
                auto& chunk = chunks.at(idx);
                chunk.faceOffset = faceOffset;
                faceOffset += chunk.faceCount;
                faceCoordsandData.insert(faceCoordsandData.end(),
                    localFaces[idx].begin(), localFaces[idx].end());
            }
        }
    }
}

void World::generateFaces(Chunk& data, std::vector<uint32_t>& allFaces, Chunk* front, Chunk* back, Chunk* right, Chunk* left, Chunk* top, Chunk* bottom, int index)
{
    for (uint8_t i = 0; i < 16;i++)
    {
        for (uint8_t j = 0; j < 16;j++)
        {
            for (uint8_t k = 0; k < 16;k++)
            {
                uint16_t id = data.getId(k, j, i);
                if (id == 0) continue;
                glm::vec3 chunkXYZ = Utils::hilbert3d_to_xyz(index, Settings::CHUNK_ORDER);
                int32_t x = chunkXYZ.x * 16 + k;
                int32_t y = chunkXYZ.y * 16 + j;
                int32_t z = chunkXYZ.z * 16 + i;
                if (((i == 0) && (front == nullptr || front->getId(k, j, 15) == 0)) || (i - 1 >= 0 && data.getId(k, j, i - 1) == 0))
                {
                    allFaces.push_back(packFace(k, j, i, 0, textureMap[id][0])); // Front
                    data.faceCount += 1;
                }
                if (((i == 15) && (back == nullptr || back->getId(k, j, 0) == 0)) || (i + 1 < 16 && data.getId(k, j, i + 1) == 0))
                {

                    allFaces.push_back(packFace(k, j, i, 1, textureMap[id][1])); // Back
                    data.faceCount += 1;
                }
                if (((k == 15) && (right == nullptr || right->getId(0, j, i) == 0)) || (k + 1 < 16 && data.getId(k + 1, j, i) == 0))
                {
                    allFaces.push_back(packFace(k, j, i, 2, textureMap[id][2])); // Right
                    data.faceCount += 1;
                }
                if (((k == 0) && (left == nullptr || left->getId(15, j, i) == 0)) || (k - 1 >= 0 && data.getId(k - 1, j, i) == 0))
                {
                    allFaces.push_back(packFace(k, j, i, 3, textureMap[id][3])); // Left
                    data.faceCount += 1;
                }
                if (((j == 15) && (top == nullptr || top->getId(k, 0, i) == 0)) || (j + 1 < 16 && data.getId(k, j + 1, i) == 0))
                {
                    allFaces.push_back(packFace(k, j, i, 4, textureMap[id][4])); // Top
                    data.faceCount += 1;
                }
                if (((j == 0) && (bottom == nullptr || bottom->getId(k, 15, i) == 0)) || (j - 1 >= 0 && data.getId(k, j - 1, i) == 0))
                {
                    allFaces.push_back(packFace(k, j, i, 5, textureMap[id][5])); // Bottom
                    data.faceCount += 1;
                }
            }
        }
    }
}

Chunk World::returnChunk(int xBlock, int yBlock, int zBlock)
{

    if ((xBlock >= Settings::BLOCK_COUNT_X || xBlock < -Settings::BLOCK_COUNT_X) || (yBlock >= Settings::BLOCK_COUNT_Y || yBlock < -Settings::BLOCK_COUNT_Y) || (zBlock >= Settings::BLOCK_COUNT_Z || zBlock < -Settings::BLOCK_COUNT_Z))
    {
        std::cout << "Out of Bounds chunk" << std::endl;
        return Chunk();
    }

    auto floorDiv = [](int a, int b) { return a / b - (a % b != 0 && (a ^ b) < 0); }; //optimisation for dividing with negative values
    int xChunk = floorDiv(xBlock, 16) + Settings::CHUNK_COUNT_X / 2; // Offset nes pradedame ne nuo 0
    int yChunk = floorDiv(yBlock, 16) + Settings::CHUNK_COUNT_Y / 2;
    int zChunk = floorDiv(zBlock, 16) + Settings::CHUNK_COUNT_Z / 2;
    return chunks[Utils::xyz_to_hilbert3d(xChunk, yChunk, zChunk, Settings::CHUNK_ORDER)];

}

int World::returnBlockID(int xBlock, int yBlock, int zBlock)
{
    if ((xBlock >= Settings::BLOCK_COUNT_X || xBlock < -Settings::BLOCK_COUNT_X) || (yBlock >= Settings::BLOCK_COUNT_Y || yBlock < -Settings::BLOCK_COUNT_Y) || (zBlock >= Settings::BLOCK_COUNT_Z || zBlock < -Settings::BLOCK_COUNT_Z))
    {
        return 0;
    }
    int xInChunk = xBlock & 15;
    int yInChunk = yBlock & 15;
    int zInChunk = zBlock & 15;
    return World::returnChunk(xBlock, yBlock, zBlock).getId(xInChunk, yInChunk, zInChunk);
}

uint32_t World::packFace(int x, int y, int z, int direction, int id)
{
    uint32_t face = 0;
    face |= (x & 0xF); // X
    face |= (y & 0xF) << 4; // Y
    face |= (z & 0xF) << 8; // Z
    face |= (direction << 12); // Face direction
    face |= (id << 15); // Id
    return face;
}
