#include "index_buffer.h"

ng::graphics::IndexBuffer::IndexBuffer(uint16 * data, uint count)
	: m_Count(count)
{
	glGenBuffers(1, &m_Handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint16), data, GL_STATIC_DRAW);
}

ng::graphics::IndexBuffer::IndexBuffer(uint32 * data, uint count)
{
	glGenBuffers(1, &m_Handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32), data, GL_STATIC_DRAW);
}

ng::graphics::IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_Handle);
}

void ng::graphics::IndexBuffer::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Handle);
}

void ng::graphics::IndexBuffer::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

uint ng::graphics::IndexBuffer::getCount()
{
	return m_Count;
}
