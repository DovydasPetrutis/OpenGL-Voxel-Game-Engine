#pragma once
#include <vector>
#include <unordered_map>
#include "Chunk.h"
inline long double PI = acos(0.0f) * 2.0L;


class Player;

struct chunkInput
{
    glm::vec4 worldPos;
    uint32_t faceCount;
    uint32_t faceOffset;
    uint32_t pad[2];
    chunkInput(glm::vec4 worldPos, uint32_t faceCount, uint32_t faceOffset) :
        worldPos(worldPos), faceCount(faceCount), faceOffset(faceOffset) {};
};

struct World
{
    int textureMap[9][6] = {

    {0    ,0    ,0    ,0    ,0    ,0    },  // ID:0 Air
    {1    ,1    ,1    ,1    ,2    ,3    },  // ID:1 Grass
    {4    ,4    ,4    ,4    ,4    ,4    },  // ID:2 Stone bricks
    {5    ,5    ,5    ,5    ,5    ,5    },  // ID:3 Diamond block
    {6    ,6    ,6    ,6    ,6    ,6    },  // ID:4 Blue wool
    {7    ,7    ,7    ,7    ,7    ,7    },  // ID:5 Emerald block
    {8    ,8    ,8    ,8    ,9    ,9    },  // ID:6 Stripped Crimson
    {10   ,10   ,10   ,10   ,10   ,10   },  // ID:7 Pink Concrete Powder
    {7    ,7    ,7    ,7    ,7    ,7    },  // ID:8 Emerald block
    };
    std::vector<uint32_t> faceCoordsandData;
    std::unordered_map<int64_t, Chunk> chunks;
    std::vector<Chunk*> activeChunks;
    std::vector<chunkInput> chunkComputeData;
    glm::vec3 previousPlayerChunkPos = glm::vec3(0.0f);
    uint64_t faceOffset = 0;
    

    void init();
    void push_chunk_vertex_data(int32_t xChunk, int32_t yChunk, int32_t zChunk);
    void push_chunk_compute_data(int32_t xChunk, int32_t yChunk, int32_t zChunk);
    void delete_chunk_vertex_data(int32_t xChunk, int32_t yChunk, int32_t zChunk);
    void delete_chunk_compute_data(int32_t xChunk, int32_t yChunk, int32_t zChunk);
    void generateFaces(Chunk& data, std::vector<uint32_t>& allFaces, Chunk* front, Chunk* back, Chunk* right, Chunk* left, Chunk* top, Chunk* bottom, int index);
    void generateChunk(int32_t xChunk,int32_t yChunk,int32_t zChunk);
    void generateChunks(Player &player);
    void activateChunk(int32_t xChunk, int32_t yChunk, int32_t zChunk);
    void deactivateChunk(int32_t xChunk, int32_t yChunk, int32_t zChunk);
    Chunk* returnChunkWithBlockCoords(int xBlock, int yBlock, int zBlock);
    Chunk* returnChunkPointerWithChunkCoords(int32_t xChunk, int32_t yChunk, int32_t zChunk, bool generateChunkIfNotFound);
    int returnBlockID(int xBlock, int yBlock, int zBlock);
    uint32_t packFace(int x, int y, int z, int direction, int id);
};

