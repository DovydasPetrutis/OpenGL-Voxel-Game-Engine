#pragma once
#include <algorithm>


class Settings
{
public:
	static inline bool CAS = true;

	// its from - to +, meaning one side is 2^21/2
	static inline const int CHUNK_COUNT_X = std::pow(2, 21);
	static inline const int CHUNK_COUNT_Y = std::pow(2, 21);
	static inline const int CHUNK_COUNT_Z = std::pow(2, 21);
	

	static inline float far_plane = 3000.0f;
	static inline float near_plane = 0.15f;
	static inline float fov = 75.0f;

	static inline int width = 2560;
	static inline int height = 1440;
	static inline float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	
	static inline float sharpness = 0.4f;
	static inline float stddev = 2.0f;
	static inline bool TAA = true;

	static inline float walkspeed = 5.0f;
	static inline float sprintspeed = 10.0f;

	// Debug
	static inline float outlineScale = 0.00f;
	static inline float outlineThickness = 0.015f;

	static inline const int RENDER_DISTANCE = 8;
};

