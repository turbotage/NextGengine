#include "resources.h"

#include "../ng_utility.h"
#include "../Vulkan/vulkan_storage.h"
#include "resource_manager.h"

// <============================= STAGING BUFFER ===============================>
/*
bool ng::StagingBuffer::hasAllocation()
{
	return m_pStagingPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}
*/

ng::StagingBuffer::~StagingBuffer()
{
	m_pStagingPage->free(*this);
}

ng::StagingBuffer::StagingBuffer(ResourceManager& manager, std::string id, uint64 size)
	: m_Manager(manager), m_ID(id), m_Size(size)
{

}








// <================================= VERTEX BUFFER ==================================>

void ng::VertexBuffer::giveDeviceAllocation(vk::CommandBuffer cb)
{
	m_Manager.giveDeviceAllocation(*this, cb);
}

void ng::VertexBuffer::giveStagingBuffer()
{
	m_Manager.giveStagingBuffer(*this);
}

void ng::VertexBuffer::setDeviceAllocationNotRequired()
{
}

void ng::VertexBuffer::setStagingBufferNotRequired()
{
}

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
		return true;
	}
	return false;
}

ng::VertexBuffer::VertexBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}











// <=================================== INDEX BUFFER ======================================>
//public
void ng::IndexBuffer::giveDeviceAllocation(vk::CommandBuffer cb)
{
	m_Manager.giveDeviceAllocation(*this, cb);
}

void ng::IndexBuffer::giveStagingBuffer()
{
	m_Manager.giveStagingBuffer(*this);
}

void ng::IndexBuffer::setDeviceAllocationNotRequired()
{
}

void ng::IndexBuffer::setStagingBufferNotRequired()
{
}

ng::IndexBuffer::~IndexBuffer()
{
	m_pIndexPage->free(*this);
}

//private
bool ng::IndexBuffer::hasDeviceAllocation()
{
	return m_pIndexPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}

bool ng::IndexBuffer::hasStagingBuffer()
{
	if (m_pStagingBuffer != nullptr) {
		return true;
	}
	return false;
}

ng::IndexBuffer::IndexBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}













// <=================================== UNIFORM BUFFER ======================================>
//public
void ng::UniformBuffer::giveDeviceAllocation(vk::CommandBuffer cb)
{
	m_Manager.giveDeviceAllocation(*this, cb);
}

void ng::UniformBuffer::giveStagingBuffer()
{
	m_Manager.giveStagingBuffer(*this);
}

void ng::UniformBuffer::setDeviceAllocationNotRequired()
{
}

void ng::UniformBuffer::setStagingBufferNotRequired()
{
}

ng::UniformBuffer::~UniformBuffer()
{
	m_pUniformPage->free(*this);
}

//private
bool ng::UniformBuffer::hasDeviceAllocation()
{
	return m_pUniformPage->getBuffer()->hasAllocation() && (m_pAllocation != nullptr);
}

bool ng::UniformBuffer::hasStagingBuffer()
{
	if (m_pStagingBuffer != nullptr) {
		return true;
	}
	return false;
}

ng::UniformBuffer::UniformBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}











// <================================== TEXTURE 2D =============================>
//public
void ng::Texture2D::giveDeviceAllocation(vk::CommandBuffer cb)
{
	m_Manager.giveDeviceAllocation(*this, cb);
}

void ng::Texture2D::giveStagingBuffer()
{
	m_Manager.giveStagingBuffer(*this);
}

void ng::Texture2D::setDeviceAllocationNotRequired()
{
}

void ng::Texture2D::setStagingBufferNotRequired()
{
}

ng::Texture2D::~Texture2D()
{
	//resets m_pVulkanTexture and m_pStagingBuffer by default i they arn't referenced anywhere
}

//private
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
		return true;
	}
	return false;
}

ng::Texture2D::Texture2D(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_ID(id)
{

}

