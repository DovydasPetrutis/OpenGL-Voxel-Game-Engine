#pragma once
#include <algorithm>


class Settings
{
public:
	static inline bool CAS = true;

	static inline const int CHUNK_COUNT_X = 10;
	static inline const int CHUNK_COUNT_Y = 10;
	static inline const int CHUNK_COUNT_Z = 10;
	static inline const int CHUNK_ORDER = std::ceil(std::log2(std::max(std::max(CHUNK_COUNT_X, CHUNK_COUNT_Y), CHUNK_COUNT_Z)));
	static inline const int BLOCK_COUNT_X = (CHUNK_COUNT_X / 2) * 16;
	static inline const int BLOCK_COUNT_Y = (CHUNK_COUNT_Y / 2) * 16;
	static inline const int BLOCK_COUNT_Z = (CHUNK_COUNT_Z / 2) * 16;
	

	static inline float far_plane = 3000.0f;
	static inline float near_plane = 0.15f;
	static inline float fov = 75.0f;

	static inline int width = 2560;
	static inline int height = 1440;
	
	static inline float sharpness = 0.4f;
	static inline float stddev = 2.0f;
	static inline bool TAA = true;

	static inline float walkspeed = 5.0f;
	static inline float sprintspeed = 10.0f;

	// Debug
	static inline float outlineScale = 0.00f;
	static inline float outlineThickness = 0.015f;
};

