#include "Chunk.h"

static std::random_device rd;
thread_local std::mt19937 gen(rd());
thread_local std::uniform_int_distribution<> distr(0,1);
Chunk::Chunk(int placeholderGenerateChunkNormal)
{
	// Generation
	for (uint16_t i = 0; i < 16;i++)
	{
		for (uint16_t j = 0;j < 16;j++)
		{
			for (uint16_t k = 0; k < 16;k++)
			{
				idBlocks[Utils::MortonEncode16t(i, j, k)] = distr(gen);
			}
		}
	}
}

int Chunk::getId(int x, int y, int z)
{
	return idBlocks[Utils::MortonEncode16t(x, y, z)];
}


// fix nes indexavimo metodas keiciasi
glm::ivec3 Chunk::returnChunkCoords()
{
	return glm::ivec3(id % Settings::CHUNK_COUNT_X_REAL, (id / Settings::CHUNK_COUNT_X_REAL) % Settings::CHUNK_COUNT_Y_REAL, id / (Settings::CHUNK_COUNT_X_REAL * Settings::CHUNK_COUNT_Y_REAL));
}

Chunk::Chunk()
{
}
