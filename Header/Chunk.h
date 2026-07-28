#pragma once
#include <iostream>
#include <cstdint>
#include <random>
#include "Utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Chunk{
public:
	uint16_t idBlocks[4096] = {{0}};
	uint64_t id = 0;
	uint16_t faceCount = 0;
	uint32_t faceOffset = 0;
	uint16_t vectorIndex = 0;
	bool isActive = false;
	Chunk(int placeholderGenerateChunkNormal);
	int getId(int x, int y, int z);
	glm::ivec3 returnChunkCoords();
	Chunk();
};