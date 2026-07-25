#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <iostream>
#include <vector>
#include "Shader.h"
#include "Settings.h"
#include "Camera.h"



class Utils
{
public:
	static float Halton(int index, int base);
	static void generateOutlineMatrices(glm::mat4& input, std::vector<glm::mat4>& output);
	static void change_frustrum(ComputeShader& cullProgram, Shader& projProgram, Camera& camera);
	static void strafeMoveCheck(GLFWwindow* window, GLenum sec1, GLenum sec2, const glm::vec3& direc, float deltaTime,Camera& camera);
	static uint16_t MortonEncode16t(uint16_t x, uint16_t y, uint16_t z);
	static uint32_t xyz_to_Morton32t(uint16_t x, uint16_t y, uint16_t z);
	static glm::ivec3 Morton32t_to_xyz(uint32_t m);
};