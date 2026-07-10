#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader 
{
public:

	GLuint id = 0;
	std::string name = "";
	std::unordered_map<std::string, int> uniformLocationCache;
	Shader();

	void use();
	void init(const std::string& vertexPath, const std::string& fragmentPath, std::string name);
	int getUniformLocation(const std::string& name);
	void setTextureBindless(std::string uniformName, GLuint64 textureHandle);
	void uniform4f(const std::string& name, float x, float y, float z, float w);
	void uniform3f(const std::string& name, float x, float y, float z);
	void uniform3f(const std::string& name, glm::vec3 vec);
	void uniform2f(const std::string& name, float x, float y);
	void uniform2f(const std::string& name, glm::vec2 vector);
	void uniform1i(const std::string& name, int x);
	void uniform1f(const std::string& name, float x);
	void uniformMatrix4fv(const std::string& name, glm::mat4 matrix);

};

class ComputeShader
{
public:
	GLuint id = 0;
	std::unordered_map<std::string, int> uniformLocationCache;

	ComputeShader();

	void use();
	void init(const std::string& path);
	int getUniformLocation(const std::string& name);
	void uniform1f(const std::string& name, float x);
	void uniformMatrix4fv(const std::string& name, glm::mat4 matrix);
	
};