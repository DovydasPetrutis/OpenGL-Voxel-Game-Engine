#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "Texture.h"

class Framebuffer
{
public:
	GLuint ID;
	std::vector<Texture> colorAttachment;
	GLuint depthAttachment;
	Framebuffer();
	void generate();
	void initRenderBuffer(std::string name, int width, int height, GLenum colorMinFilter, GLenum colorMagFilter, GLenum GPUformat, GLenum CPUformat);
	void initTexture(std::string name, int width, int height, GLenum colorMinFilter, GLenum colorMagFilter, GLenum depthMinFilter, GLenum depthMagFilter, GLenum GPUformat, GLenum CPUformat);
	void addColorAttachment(GLenum GPUformat, GLenum CPUformat, int width, int height, GLenum minFilter, GLenum magFilter);
	void use();
};