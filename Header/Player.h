#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "World.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

class Player
{
public:
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 lookAtBlockCoords = glm::vec3(0.0f);
	bool isLookingAtBlock = false;
	Player();
	Player(glm::vec3 pos) : pos(pos) {};
	void getBlockLookAtCoords(Camera& camera, World &chunks);
};