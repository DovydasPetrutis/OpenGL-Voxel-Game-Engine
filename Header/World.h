#pragma once
#include <vector>
#include <unordered_map>
#include "Chunk.h"

struct chunkInput
{
    glm::vec4 worldPos;
    uint32_t faceCount;
    uint32_t faceOffset;
    uint32_t pad[2];
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
    std::unordered_map<uint64_t, Chunk> chunks;
    std::vector<chunkInput> chunkComputeData;

    void init();
    void chunk_face_culling(std::unordered_map<uint64_t, Chunk>& chunks, std::vector<uint32_t>& faceCoordsandData);
    void generateFaces(Chunk& data, std::vector<uint32_t>& allFaces, Chunk* front, Chunk* back, Chunk* right, Chunk* left, Chunk* top, Chunk* bottom, int index);
    Chunk returnChunk(int xBlock, int yBlock, int zBlock);
    int returnBlockID(int xBlock, int yBlock, int zBlock);
    uint32_t packFace(int x, int y, int z, int direction, int id);
};

