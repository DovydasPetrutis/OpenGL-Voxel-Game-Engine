#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Texture
{
public:
	GLuint id;
	GLuint64 handle;
	bool isResident; // is immutable?
	int width, height, nrChannels;

	void generate();
	void data(const std::string& path, GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter, bool mipmaps, bool aniso, bool isResident);
	void settings(GLenum GPUFormat, GLenum CPUformat, int width, int height, GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter, bool mipmaps, bool aniso);
	Texture(GLenum GPUFormat, GLenum CPUformat, int width, int height, GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter, bool mipmaps, bool aniso);
	~Texture();
	Texture();
};