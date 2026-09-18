#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <functional>
#include <span>
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

template <typename T>
class SSBO
{
public:
	GLuint id;
	GLbitfield flags = 0;
	int size = 0;
	bool isImmutable = false;
	T* mappedBuffer = nullptr; // T is data type, like chunkInput or DrawArraysIndirect

	void init(unsigned int size,const T* data, GLbitfield usage, unsigned int bindingPoint,bool isImmutable)
	{
		glCreateBuffers(1, &id);
		this->isImmutable = isImmutable;
		this->size = size;
		flags = usage;
		if (isImmutable)
		{
			glNamedBufferStorage(id, size,NULL, usage);
		}
		else
		{
			glNamedBufferData(id, size, NULL, usage);
		}
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
		mappedBuffer = static_cast<T*>(glMapNamedBufferRange(id, 0, size, usage));
		
	}
	void update_data(std::span<const T> data)
	{
		if (data.size() > size)
		{
			std::cout << "Too much SSBO data, can't update: max count: " << size << " actual amount: " << data.size() << '\n';
			return;
		}

		uint32_t bytes = data.size_bytes();
		if (isImmutable)
		{
			std::memcpy(mappedBuffer, data.data(), bytes);
			if ((flags & GL_MAP_FLUSH_EXPLICIT_BIT) != 0)
			{
				glFlushMappedNamedBufferRange(id, 0, bytes);
			}
		}
		else
		{
			glNamedBufferSubData(id, 0, bytes, data.data());
		}
	}
	
};
 