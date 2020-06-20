#include "resources.h"

#include "../ng_utility.h"
#include "../Vulkan/vulkan_storage.h"
#include "resource_manager.h"

ng::StagingBuffer::StagingBuffer(ResourceManager& manager, std::string id)
	: m_Manager(manager), m_Id(id)
{
	auto bytes = ng::loadFile(id);

	vk::BufferCreateInfo ci{};
	ci.size = bytes.size();
	ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
	ci.sharingMode = vk::SharingMode::eExclusive;

	m_pVulkanBuffer = ngv::VulkanBuffer::make(m_Manager.vulkanDevice(), ci, true);
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

