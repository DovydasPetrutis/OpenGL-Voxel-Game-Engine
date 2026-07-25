#include "World.h"
#include "Player.h"

// morton code reikia tik vectoriams
// deactivator ir activator reikia pakeisti kadangi morton reiksia kad vectors ne is eiles
// pamastyti galbut z order geresnis
// sutvarkyti unordered map iki galo

void World::generateChunk(uint32_t x, uint32_t y, uint32_t z)
{
    uint64_t index = x + y * Settings::CHUNK_COUNT_X_REAL + Settings::CHUNK_COUNT_X_REAL * Settings::CHUNK_COUNT_Y_REAL * z;
    chunks.emplace(index, Chunk(x));
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
                const Chunk& chunk = chunks.at(Utils::xyz_to_Morton32t(k, j, i));
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
    /*
    int32_t chunkCount = chunks.size();
    const int32_t x = Settings::CHUNK_COUNT_X;
    const int32_t y = Settings::CHUNK_COUNT_Y;
    const int32_t z = Settings::CHUNK_COUNT_Z;
    auto index = [](int cx, int cy, int cz) { return Utils::xyz_to_Morton32t(cx, cy, cz);};

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
    */
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


void World::push_chunk_vertex_data(uint32_t xChunk, uint32_t yChunk, uint32_t zChunk)
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

void World::delete_chunk_vertex_data(uint32_t xChunk, uint32_t yChunk, uint32_t zChunk)
{
    // delete
    Chunk* chunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk, zChunk, false);
    if (!chunk)
    {
        std::cout << "Can't delete chunk vertex data: X: " << xChunk << " Y: " << yChunk << " Z: " << zChunk << '\n';
    }
    auto startIt = faceCoordsandData.begin() + chunk->faceOffset;
    auto endIt = startIt + chunk->faceCount;
    faceCoordsandData.erase(startIt, endIt);
    // adjust offset of other chunks
    for (int i = chunk->vectorIndex + 1; i < activeChunks.size();i++)
    {
        chunks[i].faceOffset -= chunk->faceCount;
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

// idejos kaip efektyviau activatinti, deaktyvatinti chunks

void World::generateChunks(Player& player)
{
    if (static_cast<int>(player.pos.x / 16) != previousPlayerChunkPos.x || static_cast<int>(player.pos.y / 16) != previousPlayerChunkPos.y || static_cast<int>(player.pos.z / 16) != previousPlayerChunkPos.z) // check if moved
    {
        float maxRadius = Settings::RENDER_DISTANCE;
        float maxRadiusSquared = maxRadius * maxRadius;
        glm::ivec3 playerChunk = glm::ivec3(static_cast<int>(player.pos.x) / 16, static_cast<int>(player.pos.y) / 16, static_cast<int>(player.pos.z) / 16);
        for (int z = 0; z < maxRadius;z++)
        {
            for (int y = 0; y < maxRadius;y++)
            {
                for (int x = 0; x < maxRadius;x++)
                {
                    float radiusSquared = x * x + y * y + z * z;
                    if (radiusSquared > maxRadiusSquared) continue;
                    activateChunk(playerChunk.x + x, playerChunk.y + y, playerChunk.z + z);
                }
            }
        }

        // Set the new values
        previousPlayerChunkPos.x = playerChunk.x; 
        previousPlayerChunkPos.y = playerChunk.y;
        previousPlayerChunkPos.z = playerChunk.z;
    }
}

void World::activateChunk(int xChunk, int yChunk, int zChunk)
{
    Chunk* chunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk, zChunk, true);
    if (!chunk)
    {
        std::cout << "Can't activate chunk: " << " X: " << xChunk << " Y: " << yChunk << " Z: " << zChunk << '\n';
        return;
    }
    if (chunk->isActive == true)
    {
        std::cout << "The chunk is already active: " << " X: " << xChunk << " Y: " << yChunk << " Z: " << zChunk << '\n';
        return;
    }
    chunk->vectorIndex = activeChunks.size();
    activeChunks.push_back(chunk);
    chunk->isActive = true;
}

void World::deactivateChunk(int xChunk, int yChunk, int zChunk)
{
    // swap and pop method O(1)
    Chunk* chunk = World::returnChunkPointerWithChunkCoords(xChunk, yChunk, zChunk, false);
    if (!chunk)
    {
        std::cout << "Can't deactivate chunk: " << " X: " << xChunk << " Y: " << yChunk << " Z: " << zChunk << '\n';
    }
    if (chunk->isActive == false)
    {
        std::cout << "The chunk is already deaactivated: " << " X: " << xChunk << " Y: " << yChunk << " Z: " << zChunk << '\n';
        return;
    }
    World::delete_chunk_vertex_data(xChunk, yChunk, zChunk);
    Chunk* lastChunk = activeChunks.back();
    activeChunks[chunk->vectorIndex] = lastChunk;
    lastChunk->vectorIndex = chunk->vectorIndex;
    activeChunks.pop_back();
    chunk->isActive = false;
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
        Chunk* chunk = &chunks.at(Utils::xyz_to_Morton32t(xChunk, yChunk, zChunk));
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
        return &chunks.at(Utils::xyz_to_Morton32t(xChunk, yChunk, zChunk));
    }
    catch (const std::exception& e) // if out of bounds return nullptr
    {
        if (generateChunkIfNotFound)
        {
            World::generateChunk(xChunk, yChunk, zChunk);
            return &chunks.at(Utils::xyz_to_Morton32t(xChunk, yChunk, zChunk));
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
