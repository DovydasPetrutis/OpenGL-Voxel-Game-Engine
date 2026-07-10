#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image/stb_image.h>
#include <iostream>
#include <vector>
class TextureArray
{
public:
	GLuint id;
	GLuint64 handle;
	int width, height;
	bool isResident; // is immutable?
	TextureArray();
	void generate();
	void data(int width, int height, bool isResident, std::vector<std::string> paths);
};