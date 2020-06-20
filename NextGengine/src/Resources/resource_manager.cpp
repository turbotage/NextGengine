#include "resource_manager.h"

#include "../Memory/abstract_allocators.h"
#include "resources.h"

#include "../Vulkan/vulkan_utility.h"
#include "../Vulkan/vulkan_device.h"
#include "../Vulkan/vulkan_allocator.h"
#include "../Vulkan/vulkan_storage.h"



// <==================================== RESOURCE MANAGER ==========================================>
ng::ResourceManager::ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device, ResourceStrategy strategy)
	: m_Allocator(allocator), m_Device(device), m_Strategy(strategy)
{
	// Setup Vertex Buffer Pages
	{
		m_BufferPages.hostVertexBufferPages.emplace_back();
		auto page = &m_BufferPages.hostVertexBufferPages.back();
		page->m_VertexBuffer = ngv::VulkanVertexBuffer::make(device, strategy.hostVertexBufferPageSize, true);
		page->m_Allocator = AbstractFreeListAllocator::make(strategy.hostVertexBufferPageSize);

		m_BufferPages.deviceVertexBufferPages.emplace_back();
		page = &m_BufferPages.deviceVertexBufferPages.back();
		page->m_VertexBuffer = ngv::VulkanVertexBuffer::make(device, strategy.deviceVertexBufferPageSize, false);
		page->m_Allocator = AbstractFreeListAllocator::make(strategy.deviceVertexBufferPageSize);
	}

	// Setup Index Buffer Pages
	{
		m_BufferPages.hostIndexBufferPages.emplace_back();
		auto page = &m_BufferPages.hostIndexBufferPages.back();
		page->m_IndexBuffer = ngv::VulkanIndexBuffer::make(device, strategy.hostIndexBufferPageSize, true);
		page->m_Allocator = AbstractFreeListAllocator::make(strategy.hostIndexBufferPageSize);

		m_BufferPages.deviceIndexBufferPages.emplace_back();
		page = &m_BufferPages.deviceIndexBufferPages.back();
		page->m_IndexBuffer = ngv::VulkanIndexBuffer::make(device, strategy.deviceIndexBufferPageSize, false);
		page->m_Allocator = AbstractFreeListAllocator::make(strategy.deviceIndexBufferPageSize);
	}

	// Setup Uniform Buffer Pages
	{
		m_BufferPages.hostUniformBufferPages.emplace_back();
		auto page = &m_BufferPages.hostUniformBufferPages.back();
		page->m_UniformBuffer = ngv::VulkanUniformBuffer::make(device, strategy.hostUniformBufferPageSize, true);
		page->m_Allocator = AbstractFreeListAllocator::make(strategy.hostIndexBufferPageSize);

		m_BufferPages.deviceUniformBufferPages.emplace_back();
		page = &m_BufferPages.deviceUniformBufferPages.back();
		page->m_UniformBuffer = ngv::VulkanUniformBuffer::make(device, strategy.hostUniformBufferPageSize, false);
		page->m_Allocator = AbstractFreeListAllocator::make(strategy.deviceUniformBufferPageSize);
	}





}



std::shared_ptr<ng::StagingBuffer> ng::ResourceManager::getStagingBuffer(std::string filename)
{
	auto it = m_Buffers.stagingBuffersByID.find(filename);
	if (it != m_Buffers.stagingBuffersByID.end()) {
		return it->second;
	}
	std::shared_ptr<StagingBuffer> ret = std::shared_ptr<StagingBuffer>(new StagingBuffer(*this, filename));

	for (auto& page : m_BufferPages.stagingBufferPages) {
		bool ok = page.allocate(ret);
		if (ok) {
			return;
		}
	}
#ifndef NDEBUG
	if (!shouldUseNewStagingMemory()) {
		std::runtime_error("Couldn't find any available memory for stagingBuffer and was not allowed to allocate more");
	}
#endif
	m_BufferPages.stagingBufferPages.emplace_back();
	auto page = &m_BufferPages.stagingBufferPages.back();

	uint64 pageSize = m_Strategy.stagingBufferPageSize;
	if (ret->m_Size > pageSize) {
		pageSize = ret->m_Size;
	}

	vk::BufferCreateInfo ci{};
	ci.size = pageSize;
	ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
	ci.sharingMode = vk::SharingMode::eExclusive;
	page->m_pVulkanBuffer = ngv::VulkanBuffer::make(m_Device, ci, true);
	page->m_Allocator = AbstractFreeListAllocator::make(pageSize);

#ifndef NDEBUG
	bool ok = page->allocate(ret);
	if (!ok) {
		std::runtime_error("Created new page that could not allocate");
	}
#else
	page->allocate(ret);
#endif
	m_UsedHostMemory += pageSize;

	m_Buffers.stagingBuffersByID.emplace(filename, ret);
	return ret;
}

std::shared_ptr<ng::VertexBuffer> ng::ResourceManager::getVertexBuffer(std::string filename)
{
	auto it = m_Buffers.vertexBuffersByID.find(filename);
	if (it != m_Buffers.vertexBuffersByID.end()) {
		return it->second;
	}
	std::shared_ptr<VertexBuffer> ret = std::shared_ptr<VertexBuffer>(new VertexBuffer(*this, filename));
	m_Buffers.vertexBuffersByID.emplace(filename, ret);
	return ret;
}

std::shared_ptr<ng::IndexBuffer> ng::ResourceManager::getIndexBuffer(std::string filename)
{
	auto it = m_Buffers.indexBuffersByID.find(filename);
	if (it != m_Buffers.indexBuffersByID.end()) {
		return it->second;
	}
	std::shared_ptr<IndexBuffer> ret = std::shared_ptr<IndexBuffer>(new IndexBuffer(*this, filename));
	m_Buffers.indexBuffersByID.emplace(filename, ret);
	return ret;
}

std::shared_ptr<ng::UniformBuffer> ng::ResourceManager::getUniformBuffer(std::string filename)
{
	auto it = m_Buffers.uniformBuffersByID.find(filename);
	if (it != m_Buffers.uniformBuffersByID.end()) {
		return it->second;
	}
	std::shared_ptr<UniformBuffer> ret = std::shared_ptr<UniformBuffer>(new UniformBuffer(*this, filename));
	m_Buffers.uniformBuffersByID.emplace(filename, ret);
	return ret;
}

std::shared_ptr<ng::Texture2D> ng::ResourceManager::getTexture2D(std::string filename)
{
	auto it = m_Texture2Ds.texturesByID.find(filename);
	if (it != m_Texture2Ds.texturesByID.end()) {
		return it->second;
	}
	std::shared_ptr<Texture2D> ret = std::shared_ptr<Texture2D>(new Texture2D(*this, filename));
	m_Texture2Ds.texturesByID.emplace(filename, ret);
	return ret;
}




void ng::ResourceManager::giveDeviceAllocation(std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	for (auto& page : m_BufferPages.deviceVertexBufferPages) {
		bool ok = page.allocate(vertexBuffer);
		if (ok) {
			return;
		}
	}
#ifndef NDEBUG
	if (!shouldUseNewDeviceVertexMemory()) {
		std::runtime_error("Couldn't find available memory for vertexBuffer and was not allowed to allocate more");
	}
#endif
	// We didn't find any page that could allocate, create a new one
	m_BufferPages.deviceVertexBufferPages.emplace_back();
	auto page = &m_BufferPages.deviceVertexBufferPages.back();

	uint64 pageSize = m_Strategy.deviceVertexBufferPageSize;
	if (vertexBuffer->m_Size > pageSize) {
		pageSize = vertexBuffer->m_Size;
	}

	page->m_VertexBuffer = ngv::VulkanVertexBuffer::make(m_Device, pageSize, false);
	page->m_Allocator = AbstractFreeListAllocator::make(pageSize);
	
#ifndef NDEBUG
	bool ok = page->allocate(vertexBuffer);
	if (!ok) {
		std::runtime_error("Created new page that could not allocate");
	}
#else
	page->allocate(vertexBuffer);
#endif
	m_UsedDeviceMemory += pageSize;
}

void ng::ResourceManager::giveDeviceAllocation(std::shared_ptr<IndexBuffer>& indexBuffer)
{
	for (auto& page : m_BufferPages.deviceIndexBufferPages) {
		bool ok = page.allocate(indexBuffer);
		if (ok) {
			return;
		}
	}
#ifndef NDEBUG
	if (!shouldUseNewDeviceIndexMemory()) {
		std::runtime_error("Couldn't find available memory for vertexBuffer and was not allowed to allocate more");
	}
#endif
	// We didn't find any page that could allocate, create a new one
	m_BufferPages.deviceIndexBufferPages.emplace_back();
	auto page = &m_BufferPages.deviceIndexBufferPages.back();

	uint64 pageSize = m_Strategy.deviceIndexBufferPageSize;
	if (indexBuffer->m_Size > pageSize) {
		pageSize = indexBuffer->m_Size;
	}

	page->m_IndexBuffer = ngv::VulkanIndexBuffer::make(m_Device, pageSize, false);
	page->m_Allocator = AbstractFreeListAllocator::make(pageSize);

#ifndef NDEBUG
	bool ok = page->allocate(indexBuffer);
	if (!ok) {
		std::runtime_error("Created new page that could not allocate");
	}
#else
	page->allocate(indexBuffer);
#endif // !NDEBUG
	m_UsedDeviceMemory += pageSize;
}

void ng::ResourceManager::giveDeviceAllocation(std::shared_ptr<UniformBuffer>& uniformBuffer)
{
	for (auto& page : m_BufferPages.deviceUniformBufferPages) {
		bool ok = page.allocate(uniformBuffer);
		if (ok) {
			return;
		}
	}
#ifndef NDEBUG
	if (!shouldUseNewDeviceUniformMemory()) {
		std::runtime_error("Couldn't find available memory for vertexBuffer and was not allowed to allocate more");
	}
#endif
	// We didn't find any page that could allocate, create a new one
	m_BufferPages.deviceUniformBufferPages.emplace_back();
	auto page = &m_BufferPages.deviceUniformBufferPages.back();

	uint64 pageSize = m_Strategy.deviceUniformBufferPageSize;
	if (uniformBuffer->m_Size > pageSize) {
		pageSize = uniformBuffer->m_Size;
	}

	page->m_UniformBuffer = ngv::VulkanUniformBuffer::make(m_Device, pageSize, false);
	page->m_Allocator = AbstractFreeListAllocator::make(pageSize);

#ifndef NDEBUG
	bool ok = page->allocate(uniformBuffer);
	if (!ok) {
		std::runtime_error("Created new page that could not allocate");
	}
#else
	page->allocate(uniformBuffer);
#endif // !NDEBUG
	m_UsedDeviceMemory += pageSize;
}

void ng::ResourceManager::giveDeviceAllocation(std::shared_ptr<Texture2D>& texture2D)
{
	using RD = ResourceResidencyFlag;
	if (!shouldUseNewDeviceTexture2DMemory()) {
		// We must find some similar texture2D than is available
		// check first if there are some textures that require no residency
		auto map = m_Texture2Ds.textureResidencyLists[(int)RD::eDeviceResidency][(int)RD::eNoResidency];
		for (auto& it : map) {
			if ((it.second->m_Format == texture2D->m_Format) && (it.second->m_Height == texture2D->m_Height)
				&& (it.second->m_Width == texture2D->m_Width) && (it.second->m_MipLevels == texture2D->m_MipLevels)) 
			{
				//This one can be swapped
				texture2D->m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);
				map.erase(it.first);
				m_Texture2Ds.textureResidencyLists[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D->m_Id, texture2D.get());
				return;
			}
		}
	}

	texture2D->m_pVulkanTexture = ngv::VulkanTexture2D::make(m_Device,
		texture2D->m_Width, texture2D->m_Height, texture2D->m_MipLevels, texture2D->m_Format);
	m_Allocator.giveImageAllocation(texture2D->m_pVulkanTexture);
	m_Texture2Ds.textureResidencyLists[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D->m_Id, texture2D.get());
}




const ngv::VulkanDevice& ng::ResourceManager::vulkanDevice() const
{
	return m_Device;
}




bool ng::ResourceManager::shouldUseNewStagingMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewDeviceVertexMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewHostVertexMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewDeviceIndexMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewHostIndexMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewDeviceUniformMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewHostUniformMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewDeviceTexture2DMemory()
{
	return true;
}

bool ng::ResourceManager::shouldUseNewHostTexture2DMemory()
{
	return true;
}



bool ng::StagingBufferPage::allocate(std::shared_ptr<StagingBuffer>& stagingBuffer)
{
	return false;
}

bool ng::VertexBufferPage::allocate(std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	return false;
}

bool ng::IndexBufferPage::allocate(std::shared_ptr<IndexBuffer>& indexBuffer)
{
	return false;
}

bool ng::UniformBufferPage::allocate(std::shared_ptr<UniformBuffer>& uniformBuffer)
{
	return false;
}

