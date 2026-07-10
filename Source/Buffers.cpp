#include "Buffers.h"


void VAO::bindVBO(VBOinfo VBOAndAttributes)
{
	glVertexArrayVertexBuffer(id, boundVBOs.size(), VBOAndAttributes.vbo.id, VBOAndAttributes.VBOoffset, VBOAndAttributes.stride);
	if (VBOAndAttributes.instancedDivisor != 0)
	{
		glVertexArrayBindingDivisor(id, boundVBOs.size(), VBOAndAttributes.instancedDivisor);
	}
	for (int i = 0; i < VBOAndAttributes.attributes.size(); i++)
	{
		AttributeInfo attribute = VBOAndAttributes.attributes[i];
		glVertexArrayAttribFormat(id, attribute.binding, attribute.size, attribute.type, attribute.normalize, attribute.offset);
		glVertexArrayAttribBinding(id, attribute.binding, boundVBOs.size());
		glEnableVertexArrayAttrib(id, attribute.binding);
	}
	boundVBOs.push_back(VBOAndAttributes.vbo);
}

void VAO::generate()
{
	glCreateVertexArrays(1, &id);
}

void VBO::generate()
{
	glCreateBuffers(1, &id);
}

void SSBO::generate()
{
	glCreateBuffers(1, &id);
}
