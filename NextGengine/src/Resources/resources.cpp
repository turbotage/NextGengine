#include "resources.h"

#include "../ng_utility.h"
#include "../Vulkan/vulkan_storage.h"
#include "resource_manager.h"

// <============================= STAGING BUFFER ===============================>
bool ng::StagingBuffer::hasAllocation()
{
	return m_pStagingPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}

ng::StagingBuffer::~StagingBuffer()
{
	m_pStagingPage->free(*this);
}

ng::StagingBuffer::StagingBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}








// <================================= VERTEX BUFFER ==================================>

ng::VertexBuffer::~VertexBuffer()
{
	m_pVertexPage->free(*this);
}

bool ng::VertexBuffer::hasDeviceAllocation()
{
	return m_pVertexPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}

bool ng::VertexBuffer::hasStagingBuffer()
{
	if (m_pStagingBuffer != nullptr) {
		return m_pStagingBuffer->hasAllocation();
	}
	return false;
}

ng::VertexBuffer::VertexBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}











// <=================================== INDEX BUFFER ======================================>
bool ng::IndexBuffer::hasDeviceAllocation()
{
	return m_pIndexPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}

bool ng::IndexBuffer::hasStagingBuffer()
{
	if (m_pStagingBuffer != nullptr) {
		return m_pStagingBuffer->hasAllocation();
	}
	return false;
}

ng::IndexBuffer::~IndexBuffer()
{
	m_pIndexPage->free(*this);
}

ng::IndexBuffer::IndexBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}











// <=================================== UNIFORM BUFFER ======================================>
bool ng::UniformBuffer::hasDeviceAllocation()
{
	return m_pUniformPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}

bool ng::UniformBuffer::hasStagingBuffer()
{
	if (m_pStagingBuffer != nullptr) {
		return m_pStagingBuffer->hasAllocation();
	}
	return false;
}

ng::UniformBuffer::~UniformBuffer()
{
	m_pUniformPage->free(*this);
}

ng::UniformBuffer::UniformBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}











// <================================== TEXTURE 2D =============================>
bool ng::Texture2D::hasDeviceAllocation()
{
	if (m_pVulkanTexture != nullptr) {
		return m_pVulkanTexture->hasAllocation();
	}
	return false;
}

bool ng::Texture2D::hasStagingBuffer()
{
	if (m_pStagingBuffer != nullptr) {
		return m_pStagingBuffer->hasAllocation();
	}
	return false;
}

ng::Texture2D::Texture2D(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}

