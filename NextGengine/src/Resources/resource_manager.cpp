#include "resource_manager.h"

#include "../Memory/abstract_allocators.h"
#include "resources.h"

#include "../Vulkan/vulkan_utility.h"
#include "../Vulkan/vulkan_device.h"
#include "../Vulkan/vulkan_allocator.h"
#include "../Vulkan/vulkan_storage.h"



// <==================================== RESOURCE MANAGER ==========================================>
//PUBLIC
ng::ResourceManager::ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device, ResourceStrategy strategy)
	: m_Allocator(allocator), m_Device(device), m_Strategy(strategy)
{
	// Setup Vertex Buffer Pages
	{
		m_BufferPages.vertexBufferPages.emplace_back();
		auto page = &m_BufferPages.vertexBufferPages.back();
		page->m_pVertexBuffer = ngv::VulkanVertexBuffer::make(device, strategy.hostVertexBufferPageSize, false);
		page->m_pAllocator = AbstractFreeListAllocator::make(strategy.hostVertexBufferPageSize);
	}

	// Setup Index Buffer Pages
	{
		m_BufferPages.indexBufferPages.emplace_back();
		auto page = &m_BufferPages.hostIndexBufferPages.back();
		page->m_pIndexBuffer = ngv::VulkanIndexBuffer::make(device, strategy.hostIndexBufferPageSize, false);
		page->m_pAllocator = AbstractFreeListAllocator::make(strategy.hostIndexBufferPageSize);
	}

	// Setup Uniform Buffer Pages
	{
		m_BufferPages.uniformBufferPages.emplace_back();
		auto page = &m_BufferPages.uniformBufferPages.back();
		page->m_pUniformBuffer = ngv::VulkanUniformBuffer::make(device, strategy.hostUniformBufferPageSize, false);
		page->m_pAllocator = AbstractFreeListAllocator::make(strategy.hostIndexBufferPageSize);
	}





}

const ngv::VulkanDevice& ng::ResourceManager::vulkanDevice() const
{
	return m_Device;
}


std::shared_ptr<ng::StagingBuffer> ng::ResourceManager::getStagingBuffer(std::string filename)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Staging.stagingResidencyMaps[0].find(filename);
	if (it != m_Staging.stagingResidencyMaps[0].end()) {
		return it->second;
	}
	it = m_Staging.stagingResidencyMaps[1].find(filename);
	if (it != m_Staging.stagingResidencyMaps[1].end()) {
		return it->second;
	}


	std::shared_ptr<StagingBuffer> ret = std::shared_ptr<StagingBuffer>(new StagingBuffer(*this, filename));

	for (auto& page : m_BufferPages.stagingBufferPages) {
		bool ok = page.allocate(ret);
		if (ok) {
			m_Staging.stagingResidencyMaps[0].emplace(filename, ret);
			return ret;
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
	page->m_pStagingBuffer = ngv::VulkanBuffer::make(m_Device, ci, true);
	page->m_pAllocator = AbstractFreeListAllocator::make(pageSize);
	
#ifndef NDEBUG
	bool ok = page->allocate(ret);
	if (!ok) {
		std::runtime_error("Created new page that could not allocate");
	}
#else
	page->allocate(ret);
#endif
	m_UsedHostMemory += pageSize;

	m_Staging.stagingResidencyMaps[0].emplace(filename, ret);
	return ret;
}

std::shared_ptr<ng::VertexBuffer> ng::ResourceManager::getVertexBuffer(std::string filename)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

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
	std::lock_guard<std::mutex> lock(m_Mutex);

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
	std::lock_guard<std::mutex> lock(m_Mutex);

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
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Texture2Ds.texturesByID.find(filename);
	if (it != m_Texture2Ds.texturesByID.end()) {
		return it->second;
	}
	std::shared_ptr<Texture2D> ret = std::shared_ptr<Texture2D>(new Texture2D(*this, filename));
	m_Texture2Ds.texturesByID.emplace(filename, ret);
	return ret;
}







// PRIVATE
void ng::ResourceManager::giveStagingBuffer(VertexBuffer& vertexBuffer)
{

}

void ng::ResourceManager::giveStagingBuffer(IndexBuffer& indexBuffer)
{
}

void ng::ResourceManager::giveStagingBuffer(UniformBuffer& uniformBuffer)
{
}

void ng::ResourceManager::giveStagingBuffer(Texture2D& texture2D) {
	using RD = ResourceResidencyFlagBits;
	if (!shouldUseNewStagingMemory()) {
		auto map = m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency];
		for (auto& it : map) {
			if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
				(it.second->m_Height == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
			{
				//This one can be swapped
				texture2D.m_pStagingBuffer = std::move(it.second->m_pStagingBuffer);
				
				m_Staging.stagingResidencyMaps[1].erase(texture2D.m_pStagingBuffer->m_ID);
				m_Staging.stagingResidencyMaps[0].emplace(texture2D.m_ID, texture2D.m_pStagingBuffer);

				// Upload data: TODO



			}
		}
	}
}



void ng::ResourceManager::giveDeviceAllocation(VertexBuffer& vertexBuffer)
{
	for (auto& page : m_BufferPages.vertexBufferPages) {
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
	m_BufferPages.vertexBufferPages.emplace_back();
	auto page = &m_BufferPages.vertexBufferPages.back();

	uint64 pageSize = m_Strategy.deviceVertexBufferPageSize;
	if (vertexBuffer.m_Size > pageSize) {
		pageSize = vertexBuffer.m_Size;
	}

	page->m_pVertexBuffer = ngv::VulkanVertexBuffer::make(m_Device, pageSize, false);
	page->m_pAllocator = AbstractFreeListAllocator::make(pageSize);
	
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

void ng::ResourceManager::giveDeviceAllocation(IndexBuffer& indexBuffer)
{
	for (auto& page : m_BufferPages.indexBufferPages) {
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
	m_BufferPages.indexBufferPages.emplace_back();
	auto page = &m_BufferPages.indexBufferPages.back();

	uint64 pageSize = m_Strategy.deviceIndexBufferPageSize;
	if (indexBuffer.m_Size > pageSize) {
		pageSize = indexBuffer.m_Size;
	}

	page->m_pIndexBuffer = ngv::VulkanIndexBuffer::make(m_Device, pageSize, false);
	page->m_pAllocator = AbstractFreeListAllocator::make(pageSize);

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

void ng::ResourceManager::giveDeviceAllocation(UniformBuffer& uniformBuffer)
{
	for (auto& page : m_BufferPages.uniformBufferPages) {
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
	m_BufferPages.uniformBufferPages.emplace_back();
	auto page = &m_BufferPages.uniformBufferPages.back();

	uint64 pageSize = m_Strategy.deviceUniformBufferPageSize;
	if (uniformBuffer.m_Size > pageSize) {
		pageSize = uniformBuffer.m_Size;
	}

	page->m_pUniformBuffer = ngv::VulkanUniformBuffer::make(m_Device, pageSize, false);
	page->m_pAllocator = AbstractFreeListAllocator::make(pageSize);

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

void ng::ResourceManager::giveDeviceAllocation(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	if (!shouldUseNewDeviceTexture2DMemory()) {
		// We must find some similar texture2D than is available
		// check first if there are some textures that require no residency
		auto map = m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency];
		for (auto& it : map) {
			if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height)
				&& (it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels)) 
			{
				//This one can be swapped
				texture2D.m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);


				map.erase(it.first);
				m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
				return;
			}
		}
		map = m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eStagingResidency];
		for (auto& it : map) {
			if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
				(it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels)) 
			{ 
				auto it2 = m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].find(it.first);
				if (it2 != m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].end()) {
					continue;
				}
				else { // We found a match
					texture2D.m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);

					map.erase(it.first);
					m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
					return;
				}
			}
		}
		// IF we are here we are in big trouble
#ifndef NDEBUG
		std::runtime_error("Was not allowed to allocate more memory for Texture2D and could not find any other swappable Texture2D");
#endif
	}

	texture2D.m_pVulkanTexture = ngv::VulkanTexture2D::make(m_Device,
		texture2D.m_Width, texture2D.m_Height, texture2D.m_MipLevels, texture2D.m_Format);
	m_Allocator.giveImageAllocation(texture2D.m_pVulkanTexture);
	m_Texture2Ds.textureResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
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



// PAGES

bool ng::StagingBufferPage::allocate(StagingBuffer& stagingBuffer)
{
	return false;
}

const ng::ResourceManager& ng::StagingBufferPage::getManager() const
{
	return m_Manager;
}

ng::StagingBufferPage::StagingBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{

}

bool ng::VertexBufferPage::allocate(VertexBuffer& vertexBuffer)
{
	return false;
}

const ng::ResourceManager& ng::VertexBufferPage::getManager() const
{
	return m_Manager;
}

ng::VertexBufferPage::VertexBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{
}

bool ng::IndexBufferPage::allocate(IndexBuffer& indexBuffer)
{
	return false;
}

const ng::ResourceManager& ng::IndexBufferPage::getManager() const
{
	return m_Manager;
}

ng::IndexBufferPage::IndexBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{
}

bool ng::UniformBufferPage::allocate(UniformBuffer& uniformBuffer)
{
	return false;
}

const ng::ResourceManager& ng::UniformBufferPage::getManager() const
{
	return m_Manager;
}

ng::UniformBufferPage::UniformBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{
}

