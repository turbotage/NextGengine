#include "vertex_buffer.h"

ng::graphics::VertexBuffer::VertexBuffer()
{
	glGenBuffers(1, &m_Handle);
}

ng::graphics::VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_Handle);
}

void ng::graphics::VertexBuffer::resize(uint size)
{
	m_Size = size;
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ARRAY_BUFFER, m_Size, NULL, m_Usage);
}

void ng::graphics::VertexBuffer::setData(uint size, const void * data)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
	glBufferData(GL_ARRAY_BUFFER, size, data, m_Usage);
}

void ng::graphics::VertexBuffer::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
}

void ng::graphics::VertexBuffer::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ng::graphics::VertexBuffer::release()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
}
