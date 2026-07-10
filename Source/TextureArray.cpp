#include "TextureArray.h"

TextureArray::TextureArray()
{
}

void TextureArray::generate()
{
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
}

void TextureArray::data(int width, int height, bool isResident, std::vector<std::string> paths)
{
	this->width = width;
	this->height = height;

	int layers = paths.size();
	int mipLevels = 1 + (int)std::floor(std::log2(std::max(width, height)));
	glTextureStorage3D(id, mipLevels, GL_RGBA8, width, height, layers);
	for (int i = 0; i < paths.size();i++)
	{
		int w, h, nrChannels;
		unsigned char* data = stbi_load(paths[i].c_str(), &w, &h, &nrChannels, 4);
		if (data)
		{
			glTextureSubImage3D(id, 0, 0, 0, i, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}

	}
	glGenerateTextureMipmap(id);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float maxAniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
	glTextureParameterf(id, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);

	glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
	handle = glGetTextureHandleARB(id);
	if (isResident) glMakeTextureHandleResidentARB(handle);
}

