#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>


class VBO
{
public:
	GLuint id;
	void generate();
};

struct AttributeInfo
{
	unsigned int binding;
	unsigned int size;
	GLenum type;
	GLenum normalize;
	unsigned int offset;
	AttributeInfo(unsigned int binding, unsigned int size, GLenum type, GLenum normalize, unsigned int offset) :
		binding(binding), size(size), type(type), normalize(normalize), offset(offset) {};
};

struct VBOinfo
{
	VBO &vbo;
	unsigned int stride;
	unsigned int VBOoffset;
	unsigned int instancedDivisor;
	std::vector<AttributeInfo> attributes;
	VBOinfo(VBO &vbo, unsigned int stride,unsigned int VBOoffset,unsigned int instancedDivisor ,std::vector<AttributeInfo> attributes) :
		vbo(vbo), stride(stride), VBOoffset(VBOoffset),attributes(attributes), instancedDivisor(instancedDivisor) {};
};

class VAO
{
public:
	GLuint id;
	std::vector<std::reference_wrapper<VBO>> boundVBOs;
	void generate();
	void bindVBO(VBOinfo VBOAndAttributes);
};

class SSBO
{
public:
	GLuint id;
	bool isImmutable = false;
	void generate();
	template <typename T>
	void data(unsigned int size, const T* data, GLenum usage, unsigned int bindingPoint,bool isImmutable)
	{
		this->isImmutable = isImmutable;
		if (isImmutable)
		{
			glNamedBufferStorage(id, size, data, usage);
		}
		else
		{
			glNamedBufferData(id, size, data, usage);
		}
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
		
	}
	
};
 