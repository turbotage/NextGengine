#include "vertex_array.h"
#include "vertex_buffer.h"

ng::graphics::VertexArray::VertexArray()
{

}

ng::graphics::VertexArray::~VertexArray()
{

}

ng::graphics::VertexBuffer * ng::graphics::VertexArray::getBuffer(uint index)
{
	return m_Buffers[index];
}

void ng::graphics::VertexArray::pushBuffer(VertexBuffer * buffer)
{
	m_Buffers.push_back(buffer);
}

void ng::graphics::VertexArray::bind()
{
	m_Buffers.front()->bind();
}

void ng::graphics::VertexArray::unbind()
{
	m_Buffers.front()->unbind();
}

void ng::graphics::VertexArray::draw(uint count)
{
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, NULL);
}

