#include "resource_manager.h"

#include "../Memory/abstract_allocators.h"
#include "resources.h"

#include "../Vulkan/vulkan_utility.h"
#include "../Vulkan/vulkan_device.h"
#include "../Vulkan/vulkan_allocator.h"
#include "../Vulkan/vulkan_storage.h"

#include <ktx.h>
#include <ktxvulkan.h>


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
		auto page = &m_BufferPages.indexBufferPages.back();
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

/*
std::shared_ptr<ng::StagingBuffer> ng::ResourceManager::getStagingBuffer(std::string filename)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return mGetStagingBuffer(filename);
}
*/

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

	ktxTexture* ktxTexture;
	ktxResult ktxResult;
	ktxResult = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
	ret->m_Width = ktxTexture->baseWidth;
	ret->m_Height = ktxTexture->baseHeight;
	ret->m_MipLevels = ktxTexture->numLevels;

	mGiveStagingBuffer(*ret, ktxTexture);
	m_Texture2Ds.texturesByID.emplace(filename, ret);
	return ret;
}



void ng::ResourceManager::giveStagingBuffer(VertexBuffer& vertexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveStagingBuffer(vertexBuffer);
}

void ng::ResourceManager::giveStagingBuffer(IndexBuffer& indexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveStagingBuffer(indexBuffer);
}

void ng::ResourceManager::giveStagingBuffer(UniformBuffer& uniformBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveStagingBuffer(uniformBuffer);
}

void ng::ResourceManager::giveStagingBuffer(Texture2D& texture2D) {
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveStagingBuffer(texture2D);
}



void ng::ResourceManager::giveDeviceAllocation(VertexBuffer& vertexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(vertexBuffer);
}

void ng::ResourceManager::giveDeviceAllocation(IndexBuffer& indexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(indexBuffer);
}

void ng::ResourceManager::giveDeviceAllocation(UniformBuffer& uniformBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(uniformBuffer);
}

void ng::ResourceManager::giveDeviceAllocation(Texture2D& texture2D)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(texture2D);
}

















// PRIVATE
std::shared_ptr<ng::StagingBuffer> ng::ResourceManager::mGetStagingBuffer(std::string filename)
{
	std::shared_ptr<StagingBuffer> ret = std::shared_ptr<StagingBuffer>(new StagingBuffer(*this, filename));

	std::vector<uint8> bytes;

	for (auto& page : m_BufferPages.stagingBufferPages) {
		bool ok = page.allocate(*ret);
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
	bool ok = page->allocate(*ret);
	if (!ok) {
		std::runtime_error("Created new page that could not allocate");
	}
#else
	page->allocate(*ret);
#endif
	m_UsedHostMemory += pageSize;

	m_Staging.stagingResidencyMaps[0].emplace(filename, ret);


	return ret;
}




void ng::ResourceManager::mGiveStagingBuffer(VertexBuffer& vertexBuffer)
{
	if (vertexBuffer.hasStagingBuffer()) {
		m_Staging.stagingResidencyMaps[0].emplace(vertexBuffer.m_ID, vertexBuffer.m_pStagingBuffer);
		m_Staging.stagingResidencyMaps[1].erase(vertexBuffer.m_ID);
		return;
	}
	else {
		vertexBuffer.m_pStagingBuffer = mGetStagingBuffer(vertexBuffer.m_ID);
	}
}

void ng::ResourceManager::mGiveStagingBuffer(IndexBuffer& indexBuffer)
{
	if (indexBuffer.hasStagingBuffer()) {
		m_Staging.stagingResidencyMaps[0].emplace(indexBuffer.m_ID, indexBuffer.m_pStagingBuffer);
		m_Staging.stagingResidencyMaps[1].erase(indexBuffer.m_ID);
		return;
	}
	else {
		indexBuffer.m_pStagingBuffer = mGetStagingBuffer(indexBuffer.m_ID);
	}
}

void ng::ResourceManager::mGiveStagingBuffer(UniformBuffer& uniformBuffer)
{
	if (uniformBuffer.hasStagingBuffer()) {
		m_Staging.stagingResidencyMaps[0].emplace(uniformBuffer.m_ID, uniformBuffer.m_pStagingBuffer);
		m_Staging.stagingResidencyMaps[1].erase(uniformBuffer.m_ID);
		return;
	}
	else {
		uniformBuffer.m_pStagingBuffer = mGetStagingBuffer(uniformBuffer.m_ID);
	}
}

void ng::ResourceManager::mGiveStagingBuffer(Texture2D& texture2D, ktxTexture* ktxTexture)
{
	if (texture2D.hasStagingBuffer()) {
		m_Staging.stagingResidencyMaps[0].emplace(texture2D.m_ID, texture2D.m_pStagingBuffer);
		m_Staging.stagingResidencyMaps[1].erase(texture2D.m_ID);
		return;
	}
	else {
		using RD = ResourceResidencyFlagBits;
		texture2D.m_pStagingBuffer = std::shared_ptr<StagingBuffer>(new StagingBuffer(*this, texture2D.m_ID));

		for (auto& page : m_BufferPages.stagingBufferPages) {
			bool ok = page.allocate(*texture2D.m_pStagingBuffer);
			if (ok) {
				m_Staging.stagingResidencyMaps[0].emplace(texture2D.m_ID, texture2D.m_pStagingBuffer);
				m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);

				// Upload data: TODO
				{
					ktxResult result;
					if (ktxTexture == nullptr) {
						result = ktxTexture_CreateFromNamedFile(texture2D.m_ID.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
					}
					ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
					ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

					//map, copy unmap
					void* data = texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->map(
						texture2D.m_pStagingBuffer->m_pAllocation->getOffset(), texture2D.m_pStagingBuffer->m_Size);
					memcpy(data, ktxTextureData, ktxTextureSize);
					texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->unmap();
				}

				return;
			}
		}

		if (!shouldUseNewStagingMemory()) {
			auto map = m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency];
			for (auto& it : map) {
				if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
					(it.second->m_Height == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
				{
					//This one can be swapped
					texture2D.m_pStagingBuffer = std::move(it.second->m_pStagingBuffer);

					m_Staging.stagingResidencyMaps[1].erase(texture2D.m_pStagingBuffer->m_ID);
					texture2D.m_pStagingBuffer->m_ID = texture2D.m_ID;
					m_Staging.stagingResidencyMaps[0].emplace(texture2D.m_ID, texture2D.m_pStagingBuffer);

					// Upload data: TODO
					{
						ktxResult result;
						if (ktxTexture == nullptr) {
							result = ktxTexture_CreateFromNamedFile(texture2D.m_ID.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
						}
						ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
						ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);


						//map, copy unmap
						void* data = texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->map(
							texture2D.m_pStagingBuffer->m_pAllocation->getOffset(), texture2D.m_pStagingBuffer->m_Size);
						memcpy(data, ktxTextureData, ktxTextureSize);
						texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->unmap();
					}

					//
					map.erase(it.first);
					m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
					return;
				}
			}
			map = m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eDeviceResidency];
			for (auto& it : map) {
				if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
					(it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
				{
					auto it2 = m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].find(it.first);
					if (it2 != m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].end()) {
						continue;
					}
					else {
						texture2D.m_pStagingBuffer = it.second->m_pStagingBuffer;

						m_Staging.stagingResidencyMaps[1].erase(texture2D.m_pStagingBuffer->m_ID);
						texture2D.m_pStagingBuffer->m_ID = texture2D.m_ID;
						m_Staging.stagingResidencyMaps[0].emplace(texture2D.m_ID, texture2D.m_pStagingBuffer);

						// Upload data: TODO
						{
							ktxResult result;
							if (ktxTexture == nullptr) {
								result = ktxTexture_CreateFromNamedFile(texture2D.m_ID.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
							}
							ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
							ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);


							//map, copy unmap
							void* data = texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->map(
								texture2D.m_pStagingBuffer->m_pAllocation->getOffset(), texture2D.m_pStagingBuffer->m_Size);
							memcpy(data, ktxTextureData, ktxTextureSize);
							texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->unmap();
						}

						//
						map.erase(it.first);
						m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
						return;
					}
				}
			}
		}

		m_BufferPages.stagingBufferPages.emplace_back();
		auto page = &m_BufferPages.stagingBufferPages.back();

		uint64 pageSize = m_Strategy.stagingBufferPageSize;
		if (texture2D.m_pStagingBuffer->m_Size > pageSize) {
			pageSize = texture2D.m_pStagingBuffer->m_Size;
		}
		
		vk::BufferCreateInfo ci{};
		ci.size = pageSize;
		ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
		ci.sharingMode = vk::SharingMode::eExclusive;
		page->m_pStagingBuffer = ngv::VulkanBuffer::make(m_Device, ci, true);
		page->m_pAllocator = ng::AbstractFreeListAllocator::make(pageSize);

#ifndef NDEBUG
		bool ok = page->allocate(*texture2D.m_pStagingBuffer);
		if (!ok) {
			std::runtime_error("Created new page that could not allocate");
		}
#else
		page->allocate(*texture2D.m_pStagingBuffer);
#endif // !NDEBUG
		m_UsedHostMemory += pageSize;

		m_Staging.stagingResidencyMaps[0].emplace(texture2D.m_ID, texture2D.m_pStagingBuffer);
		m_Texture2Ds.textureResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
	}

}




void ng::ResourceManager::mGiveDeviceAllocation(VertexBuffer& vertexBuffer)
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

void ng::ResourceManager::mGiveDeviceAllocation(IndexBuffer& indexBuffer)
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

void ng::ResourceManager::mGiveDeviceAllocation(UniformBuffer& uniformBuffer)
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

void ng::ResourceManager::mGiveDeviceAllocation(Texture2D& texture2D)
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
// Staging
bool ng::StagingBufferPage::allocate(StagingBuffer& stagingBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	stagingBuffer.m_pAllocation = m_pAllocator->allocate(stagingBuffer.m_Size, 1);
	if (stagingBuffer.m_pAllocation == nullptr) {
		return false;
	}
	stagingBuffer.m_pStagingPage = this;
	return true;
}

void ng::StagingBufferPage::free(StagingBuffer& stagingBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	m_pAllocator->free(std::move(stagingBuffer.m_pAllocation));
	stagingBuffer.m_pStagingPage = nullptr;
}

const ng::ResourceManager& ng::StagingBufferPage::getManager() const
{
	return m_Manager;
}

ng::raw_ptr<ngv::VulkanBuffer> ng::StagingBufferPage::getBuffer()
{
	return m_pStagingBuffer.get();
}

ng::StagingBufferPage::StagingBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{

}






// VertexBuffer
bool ng::VertexBufferPage::allocate(VertexBuffer& vertexBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	vertexBuffer.m_pAllocation = m_pAllocator->allocate(vertexBuffer.m_Size, 1);
	if (vertexBuffer.m_pAllocation == nullptr) {
		return false;
	}
	vertexBuffer.m_pVertexPage = this;
	return true;
}

void ng::VertexBufferPage::free(VertexBuffer& vertexBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	m_pAllocator->free(std::move(vertexBuffer.m_pAllocation));
	vertexBuffer.m_pVertexPage = nullptr;
}

const ng::ResourceManager& ng::VertexBufferPage::getManager() const
{
	return m_Manager;
}

ng::raw_ptr<ngv::VulkanVertexBuffer> ng::VertexBufferPage::getBuffer()
{
	return m_pVertexBuffer.get();
}

ng::VertexBufferPage::VertexBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{
}





// IndexBuffer
bool ng::IndexBufferPage::allocate(IndexBuffer& indexBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	indexBuffer.m_pAllocation = m_pAllocator->allocate(indexBuffer.m_Size, 1);
	if (indexBuffer.m_pAllocation == nullptr) {
		return false;
	}
	indexBuffer.m_pIndexPage = this;
	return true;
}

void ng::IndexBufferPage::free(IndexBuffer& indexBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	m_pAllocator->free(std::move(indexBuffer.m_pAllocation));
	indexBuffer.m_pIndexPage = nullptr;
}

const ng::ResourceManager& ng::IndexBufferPage::getManager() const
{
	return m_Manager;
}

ng::raw_ptr<ngv::VulkanIndexBuffer> ng::IndexBufferPage::getBuffer()
{
	return m_pIndexBuffer.get();
}

ng::IndexBufferPage::IndexBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{
}






// UniformBuffer
bool ng::UniformBufferPage::allocate(UniformBuffer& uniformBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	uniformBuffer.m_pAllocation = m_pAllocator->allocate(uniformBuffer.m_Size, 1);
	if (uniformBuffer.m_pAllocation == nullptr) {
		return false;
	}
	uniformBuffer.m_pUniformPage = this;
	return true;
}

void ng::UniformBufferPage::free(UniformBuffer& uniformBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	m_pAllocator->free(std::move(uniformBuffer.m_pAllocation));
	uniformBuffer.m_pUniformPage = nullptr;
}

const ng::ResourceManager& ng::UniformBufferPage::getManager() const
{
	return m_Manager;
}

ng::raw_ptr<ngv::VulkanUniformBuffer> ng::UniformBufferPage::getBuffer()
{
	return m_pUniformBuffer.get();
}

ng::UniformBufferPage::UniformBufferPage(const ResourceManager& manager)
	: m_Manager(manager)
{
}

