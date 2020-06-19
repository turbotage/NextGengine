#include "resources.h"


ng::StagingBuffer::StagingBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_Id(id)
{
}

ng::VertexBuffer::VertexBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_Id(id)
{

}

ng::IndexBuffer::IndexBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_Id(id)
{

}

ng::UniformBuffer::UniformBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_Id(id)
{

}

ng::Texture2D::Texture2D(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_Id(id)
{

}

