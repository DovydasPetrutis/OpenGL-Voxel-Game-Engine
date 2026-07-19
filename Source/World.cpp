#include "World.h"
#include "Player.h"

void World::generateChunk(uint32_t x, uint32_t y, uint32_t z)
{
    chunks.emplace(Utils::xyz_to_hilbert3d(x, y, z, Settings::CHUNK_ORDER), Chunk(x));
}


void World::init()
{

    for (int i = -Settings::CHUNK_COUNT_Z / 2; i < Settings::CHUNK_COUNT_Z / 2; i++)
    {
        for (int j = -Settings::CHUNK_COUNT_Y / 2; j < Settings::CHUNK_COUNT_Y / 2; j++)
        {
            for (int k = -Settings::CHUNK_COUNT_X / 2; k < Settings::CHUNK_COUNT_X / 2; k++)
            {
                generateChunk(k,j,i);
            }
        }
    }

    chunk_face_culling();

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
    previousPlayerPos = glm::vec3(0.0f);

}

void World::chunk_face_culling()
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


void World::chunk_face_culling(uint32_t xChunk, uint32_t yChunk, uint32_t zChunk)
{
    Chunk* chunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk, zChunk,true);
    if (chunk == nullptr)
    {
        std::cout << "Chunk not Found: X: " << xChunk << " Y: " << yChunk << " Z: " << zChunk << '\n';
        return;
    }
    Chunk* rightChunk = World::returnChunkPointerWithChunkCoords(xChunk + 1, yChunk, zChunk, true);
    Chunk* leftChunk = World::returnChunkPointerWithChunkCoords(xChunk - 1, yChunk, zChunk, true);
    Chunk* topChunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk + 1, zChunk, true);
    Chunk* bottomChunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk - 1, zChunk, true);
    Chunk* frontChunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk, zChunk + 1, true);
    Chunk* behindChunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk, zChunk - 1, true);
    // Optimisation, preocmpute bool values

    for (uint8_t z = 0; z < 16;z++)
    {
        for (uint8_t y = 0; y < 16;y++)
        {
            for (uint8_t x = 0; x < 16;x++)
            {
                uint16_t id = chunk->getId(x, y, z);
                // Front 
                {
                    bool showFace = false;
                    if (z != 0) showFace = (chunk->getId(x, y, z - 1) == 0);
                    else        showFace = (frontChunk->getId(x, y, 15) == 0);

                    if (showFace)
                    {
                        faceCoordsandData.push_back(packFace(x, y, z, 0, textureMap[id][0]));
                        chunk->faceCount += 1;
                    }
                }
                // Back 
                {
                    bool showFace = false;
                    if (z != 15) showFace = (chunk->getId(x, y, z + 1) == 0);
                    else         showFace = (behindChunk->getId(x, y, 0) == 0);

                    if (showFace)
                    {
                        faceCoordsandData.push_back(packFace(x, y, z, 1, textureMap[id][1]));
                        chunk->faceCount += 1;
                    }
                }
                // Right
                {
                    bool showFace = false;
                    if (x != 15) showFace = (chunk->getId(x + 1, y, z) == 0);
                    else         showFace = (rightChunk->getId(0, y, z) == 0);

                    if (showFace)
                    {
                        faceCoordsandData.push_back(packFace(x, y, z, 2, textureMap[id][2]));
                        chunk->faceCount += 1;
                    }
                }
                // Left
                {
                    bool showFace = false;
                    if (x != 0) showFace = (chunk->getId(x - 1, y, z) == 0);
                    else         showFace = (leftChunk->getId(15, y, z) == 0);

                    if (showFace)
                    {
                        faceCoordsandData.push_back(packFace(x, y, z, 3, textureMap[id][3]));
                        chunk->faceCount += 1;
                    }
                }
                // Top
                {
                    bool showFace = false;
                    if (x != 0) showFace = (chunk->getId(x, y + 1, z) == 0);
                    else         showFace = (topChunk->getId(x, 0, z) == 0);

                    if (showFace)
                    {
                        faceCoordsandData.push_back(packFace(x, y, z, 4, textureMap[id][4]));
                        chunk->faceCount += 1;
                    }
                }
                // Bottom
                {
                    bool showFace = false;
                    if (x != 0) showFace = (chunk->getId(x, y - 1, z) == 0);
                    else         showFace = (bottomChunk->getId(x, 15, z) == 0);

                    if (showFace)
                    {
                        faceCoordsandData.push_back(packFace(x, y, z, 5, textureMap[id][5]));
                        chunk->faceCount += 1;
                    }
                }
            }
        }
    }
    chunk->faceOffset = faceOffset;
    faceOffset += chunk->faceCount;

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



void World::generateChunks(Player& player)
{
    if (static_cast<int>(player.pos.x) != previousPlayerPos.x || static_cast<int>(player.pos.y) != previousPlayerPos.y || static_cast<int>(player.pos.z) != previousPlayerPos.z) // check if moved
    {
        float maxRadius = Settings::RENDER_DISTANCE;
        float maxRadiusSquared = maxRadius * maxRadius;
        for (int z = 0; z < maxRadius;z++)
        {
            for (int y = 0; y < maxRadius;y++)
            {
                for (int x = 0; x < maxRadius;x++)
                {
                    float radiusSquared = x * x + y * y + z * z;
                    if (radiusSquared > maxRadiusSquared) continue;


                }
            }
        }

        // Set the new values
        previousPlayerPos.x = static_cast<int>(player.pos.x); 
        previousPlayerPos.y = static_cast<int>(player.pos.y);
        previousPlayerPos.z = static_cast<int>(player.pos.z);
    }
}


Chunk* World::returnChunkWithBlockCoords(int xBlock, int yBlock, int zBlock)
{
    /*
    if ((xBlock >= Settings::BLOCK_COUNT_X || xBlock < -Settings::BLOCK_COUNT_X) || (yBlock >= Settings::BLOCK_COUNT_Y || yBlock < -Settings::BLOCK_COUNT_Y) || (zBlock >= Settings::BLOCK_COUNT_Z || zBlock < -Settings::BLOCK_COUNT_Z))
    {
        std::cout << "Out of Bounds chunk" << std::endl;
        return Chunk();
    }
    */
    auto floorDiv = [](int a, int b) { return a / b - (a % b != 0 && (a ^ b) < 0); }; //optimisation for dividing with negative values
    int xChunk = floorDiv(xBlock, 16); // Offset nes pradedame ne nuo 0
    int yChunk = floorDiv(yBlock, 16);
    int zChunk = floorDiv(zBlock, 16);
    try
    {
        Chunk* chunk = &chunks.at(Utils::xyz_to_hilbert3d(xChunk, yChunk, zChunk, Settings::CHUNK_ORDER));
        return chunk;
    }
    catch (const std::exception& e)
    {
        return nullptr;
    }

}

Chunk* World::returnChunkPointerWithChunkCoords(int xChunk, int yChunk, int zChunk,bool generateChunkIfNotFound)
{
    try
    {
        return &chunks.at(Utils::xyz_to_hilbert3d(xChunk, yChunk, zChunk, Settings::CHUNK_ORDER));
    }
    catch (const std::exception& e) // if out of bounds return nullptr
    {
        if (generateChunkIfNotFound)
        {
            World::generateChunk(xChunk, yChunk, zChunk);
            return &chunks.at(Utils::xyz_to_hilbert3d(xChunk, yChunk, zChunk, Settings::CHUNK_ORDER));
        }
        else
        {
            return nullptr;
        }
    }
}

int World::returnBlockID(int xBlock, int yBlock, int zBlock)
{
    /*
    if ((xBlock >= Settings::BLOCK_COUNT_X || xBlock < -Settings::BLOCK_COUNT_X) || (yBlock >= Settings::BLOCK_COUNT_Y || yBlock < -Settings::BLOCK_COUNT_Y) || (zBlock >= Settings::BLOCK_COUNT_Z || zBlock < -Settings::BLOCK_COUNT_Z))
    {
        return 0;
    }
    */
    std::cout << chunks.size() << std::endl;
    int xInChunk = xBlock & 15;
    int yInChunk = yBlock & 15;
    int zInChunk = zBlock & 15;
    Chunk* chunk = returnChunkWithBlockCoords(xBlock, yBlock, zBlock);
    if (!chunk) return 0; // treat ungenerated/out-of-range as air

    return chunk->getId(xInChunk, yInChunk, zInChunk);
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
