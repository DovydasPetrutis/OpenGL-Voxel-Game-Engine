#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

void Texture::generate()
{
	glCreateTextures(GL_TEXTURE_2D, 1, &id);
}

void Texture::data(const std::string& path, GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter, bool mipmaps, bool aniso, bool isResident)
{
	this->isResident = isResident;
	int forcedChannels = 4;
	GLubyte* data = stbi_load(path.c_str(), &width, &height, &nrChannels, forcedChannels);
	nrChannels = forcedChannels;
	if (data)
	{
		int levels = 1;
		if (mipmaps) {
			levels = int((std::floor(std::log2(std::max(width, height)))) + 1);
		}
		if (nrChannels == 4)
		{
			glTextureStorage2D(id, levels, GL_RGBA8, width, height);
			glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Exotic NrChannels texture: " << path << " can't be read!" << std::endl;
		}
		if (mipmaps) glGenerateTextureMipmap(id);
		if (aniso)
		{
			float maxAniso;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
			glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
		}
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap_s);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap_t);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
		handle = glGetTextureHandleARB(id);
		if (isResident) glMakeTextureHandleResidentARB(handle);
		stbi_image_free(data);
	}
}

void Texture::settings(GLenum GPUFormat, GLenum CPUformat, int width, int height, GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter, bool mipmaps, bool aniso)
{
	this->width = width;
	this->height = height;
	int levels = 1;
	if (mipmaps) {
		levels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
	}
	glTextureStorage2D(id, levels, GPUFormat, width, height);
	//glTexImage2D(GL_TEXTURE_2D, 0, GPUFormat, width, height, 0, CPUformat, GL_UNSIGNED_BYTE, NULL);
	if (mipmaps) glGenerateTextureMipmap(id);
	if (aniso)
	{
		float maxAniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
		glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
	}
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap_s);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap_t);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
}


Texture::Texture(GLenum GPUFormat,GLenum CPUformat,int width, int height,GLenum wrap_s, GLenum wrap_t, GLenum min_filter, GLenum mag_filter, bool mipmaps, bool aniso)
{
	this->width = width;
	this->height = height;
	glCreateTextures(GL_TEXTURE_2D, 1,&id);
	int levels = 1;
	if (mipmaps) {
		levels = static_cast<int>(std::floor(std::log2(std::max(width, height)))) + 1;
	}
	glTextureStorage2D(id, levels, GPUFormat, width, height);
	//glTexImage2D(GL_TEXTURE_2D, 0, GPUFormat, width, height, 0, CPUformat, GL_UNSIGNED_BYTE, NULL);
	if (mipmaps) glGenerateTextureMipmap(id);
	if (aniso)
	{
		float maxAniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
		glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY,maxAniso);
	}
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap_s);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap_t);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, mag_filter);
}

Texture::~Texture()
{
	if(isResident) glMakeTextureHandleNonResidentARB(handle);
	glDeleteTextures(1, &id);
}

Texture::Texture()
{
}
