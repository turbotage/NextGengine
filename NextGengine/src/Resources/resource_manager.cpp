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
	// Setup Staging Buffer Pages
	{
		m_BufferPages.stagingBufferPages.emplace_back(*this);
		auto page = &m_BufferPages.stagingBufferPages.back();
		vk::BufferCreateInfo ci{};
		ci.size = m_Strategy.stagingBufferPageSize;
		ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
		ci.sharingMode = vk::SharingMode::eExclusive;
		page->m_pStagingBuffer = ngv::VulkanBuffer::make(device, ci, true);
		allocator.giveBufferAllocation(*page->m_pStagingBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::make(ci.size);
	}
	// Setup Vertex Buffer Pages
	{
		m_BufferPages.vertexBufferPages.emplace_back(*this);
		auto page = &m_BufferPages.vertexBufferPages.back();
		page->m_pVertexBuffer = ngv::VulkanVertexBuffer::make(device, strategy.hostVertexBufferPageSize, false);
		allocator.giveBufferAllocation(*page->m_pVertexBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::make(strategy.hostVertexBufferPageSize);
	}

	// Setup Index Buffer Pages
	{
		m_BufferPages.indexBufferPages.emplace_back(*this);
		auto page = &m_BufferPages.indexBufferPages.back();
		page->m_pIndexBuffer = ngv::VulkanIndexBuffer::make(device, strategy.hostIndexBufferPageSize, false);
		allocator.giveBufferAllocation(*page->m_pIndexBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::make(strategy.hostIndexBufferPageSize);
	}

	// Setup Uniform Buffer Pages
	{
		m_BufferPages.uniformBufferPages.emplace_back(*this);
		auto page = &m_BufferPages.uniformBufferPages.back();
		page->m_pUniformBuffer = ngv::VulkanUniformBuffer::make(device, strategy.hostUniformBufferPageSize, false);
		allocator.giveBufferAllocation(*page->m_pUniformBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::make(strategy.hostIndexBufferPageSize);
	}

}

const ngv::VulkanDevice& ng::ResourceManager::vulkanDevice() const
{
	return m_Device;
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

	auto it = m_Textures.texture2DsByID.find(filename);
	if (it != m_Textures.texture2DsByID.end()) {
		return it->second;
	}
	std::shared_ptr<Texture2D> ret = std::shared_ptr<Texture2D>(new Texture2D(*this, filename));

	mGiveStagingBuffer(*ret);
	m_Textures.texture2DsByID.emplace(filename, ret);
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



void ng::ResourceManager::giveDeviceAllocation(VertexBuffer& vertexBuffer, vk::CommandBuffer cb)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(vertexBuffer, cb);
}

void ng::ResourceManager::giveDeviceAllocation(IndexBuffer& indexBuffer, vk::CommandBuffer cb)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(indexBuffer, cb);
}

void ng::ResourceManager::giveDeviceAllocation(UniformBuffer& uniformBuffer, vk::CommandBuffer cb)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(uniformBuffer, cb);
}

void ng::ResourceManager::giveDeviceAllocation(Texture2D& texture2D, vk::CommandBuffer cb)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveDeviceAllocation(texture2D, cb);
}

















// PRIVATE
//should always be used in conjunction with uploadToStagingBuffer, no resource should be able to have a staging buffer that doesn't hold it's stagingData
std::shared_ptr<ng::StagingBuffer> ng::ResourceManager::mGetStagingBuffer(std::string id, uint64 size)
{
	std::shared_ptr<StagingBuffer> ret = std::shared_ptr<StagingBuffer>(new StagingBuffer(*this, id, size));

	for (auto& page : m_BufferPages.stagingBufferPages) {
		bool ok = page.allocate(*ret);
		if (ok) {
			return ret;
		}
	}
#ifndef NDEBUG
	if (!mShouldUseNewStagingMemory()) {
		// TODO: free memory from other staging allocations that arn't needed so that this one can allocate
		std::runtime_error("Couldn't find any available memory for stagingBuffer and was not allowed to allocate more");
	}
#endif
	m_BufferPages.stagingBufferPages.emplace_back(*this);
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
	return ret;
}

void ng::ResourceManager::mUploadToStagingBuffer(StagingBuffer& buffer, void* data)
{
	void* mapped = buffer.m_pStagingPage->m_pStagingBuffer->map(
		buffer.m_pAllocation->getOffset(), buffer.m_Size);
	memcpy(mapped, data, buffer.m_Size);
	buffer.m_pStagingPage->m_pStagingBuffer->unmap();
}


void ng::ResourceManager::mGiveStagingBuffer(VertexBuffer& vertexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if (vertexBuffer.hasStagingBuffer()) {
		m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
		m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(vertexBuffer.m_ID);
		return;
	}
	else {
		auto bytes = ng::loadNGFile(vertexBuffer.m_ID);
		vertexBuffer.m_Size = bytes.size();
		vertexBuffer.m_pStagingBuffer = mGetStagingBuffer(vertexBuffer.m_ID, vertexBuffer.m_Size);
		mUploadToStagingBuffer(*vertexBuffer.m_pStagingBuffer, bytes.data());
	}
}

void ng::ResourceManager::mGiveStagingBuffer(IndexBuffer& indexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if (indexBuffer.hasStagingBuffer()) {
		m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(indexBuffer.m_ID, &indexBuffer);
		m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(indexBuffer.m_ID);
		return;
	}
	else {
		auto bytes = ng::loadNGFile(indexBuffer.m_ID);
		indexBuffer.m_Size = bytes.size();
		indexBuffer.m_pStagingBuffer = mGetStagingBuffer(indexBuffer.m_ID, indexBuffer.m_Size);
		mUploadToStagingBuffer(*indexBuffer.m_pStagingBuffer, bytes.data());
	}
}

void ng::ResourceManager::mGiveStagingBuffer(UniformBuffer& uniformBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if (uniformBuffer.hasStagingBuffer()) {
		m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
		m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(uniformBuffer.m_ID);
		return;
	}
	else {
		auto bytes = ng::loadNGFile(uniformBuffer.m_ID);
		uniformBuffer.m_Size = bytes.size();
		uniformBuffer.m_pStagingBuffer = mGetStagingBuffer(uniformBuffer.m_ID, uniformBuffer.m_Size);
		mUploadToStagingBuffer(*uniformBuffer.m_pStagingBuffer, bytes.data());
	}
}

void ng::ResourceManager::mGiveStagingBuffer(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	if (texture2D.hasStagingBuffer()) {
		m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
		m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(texture2D.m_ID);
		return;
	}
	else {
		using RD = ResourceResidencyFlagBits;
		ktxResult result;
		result = ktxTexture_CreateFromNamedFile(texture2D.m_ID.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture2D.m_KTXTexture);
		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(texture2D.m_KTXTexture);
		ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(texture2D.m_KTXTexture);
		texture2D.m_Width = texture2D.m_KTXTexture->baseWidth;
		texture2D.m_Height = texture2D.m_KTXTexture->baseHeight;
		texture2D.m_MipLevels = texture2D.m_KTXTexture->numLevels;
		texture2D.m_Format = (vk::Format)ktxTexture_GetVkFormat(texture2D.m_KTXTexture);
#ifndef NDEBUG
		if (texture2D.m_Format == vk::Format::eUndefined) {
			std::runtime_error("Texture had undefined format");
		}
#endif
		texture2D.m_pStagingBuffer = std::shared_ptr<StagingBuffer>(new StagingBuffer(*this, texture2D.m_ID, ktxTextureSize));
		
		// Basically reinvents mGetStagingBuffer
		for (auto& page : m_BufferPages.stagingBufferPages) {
			bool ok = page.allocate(*texture2D.m_pStagingBuffer);
			if (ok) {
				m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);

				// Upload data:
				mUploadToStagingBuffer(*texture2D.m_pStagingBuffer, (void*)ktxTextureData);

				return;
			}
		}

		// We shouldn't trow a runtime error here, instead try to find a texture that doesn't need it's staging buffer anymore
		if (!mShouldUseNewStagingMemory()) {
			auto map = m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency];
			for (auto& it : map) {
				if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
					(it.second->m_Height == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
				{
					// Swap staging buffer
					texture2D.m_pStagingBuffer = std::move(it.second->m_pStagingBuffer);
					texture2D.m_pStagingBuffer->m_ID = texture2D.m_ID;

					map.erase(it.first);
					m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);

					// Upload data:
					mUploadToStagingBuffer(*texture2D.m_pStagingBuffer, (void*)ktxTextureData);

					return;
				}
			}
			map = m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eDeviceResidency];
			for (auto& it : map) {
				if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
					(it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
				{
					auto it2 = m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].find(it.first);
					if (it2 != m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].end()) {
						continue;
					}
					else {
						texture2D.m_pStagingBuffer = std::move(it.second->m_pStagingBuffer);
						texture2D.m_pStagingBuffer->m_ID = texture2D.m_ID;

						map.erase(it.first);
						m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);

						// Upload data:
						mUploadToStagingBuffer(*texture2D.m_pStagingBuffer, (void*)ktxTextureData);

						return;
					}
				}
			}
#ifndef NDEBUG
			std::runtime_error("Was not allowed to allocate more staging memory for Texture2D and could not find any other swappable Texture2D");
#endif // !NDEBUG


		}
		else { // We can use new staging memory so make a new page and allocate from it
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

			m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);

			// Upload data:
			mUploadToStagingBuffer(*texture2D.m_pStagingBuffer, (void*)ktxTextureData);
		}
	}
}




void ng::ResourceManager::mUploadToDevice(VertexBuffer& buffer, vk::CommandBuffer cb)
{
	vk::BufferCopy bc{};
	bc.srcOffset = buffer.m_pStagingBuffer->m_pAllocation->getOffset();
	bc.dstOffset = buffer.m_pAllocation->getOffset();
	bc.size = buffer.m_Size;
	cb.copyBuffer(buffer.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->buffer(), buffer.m_pVertexPage->m_pVertexBuffer->buffer(), bc);
}

void ng::ResourceManager::mUploadToDevice(IndexBuffer& buffer, vk::CommandBuffer cb)
{
	vk::BufferCopy bc{};
	bc.srcOffset = buffer.m_pStagingBuffer->m_pAllocation->getOffset();
	bc.dstOffset = buffer.m_pAllocation->getOffset();
	bc.size = buffer.m_Size;
	cb.copyBuffer(buffer.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->buffer(), buffer.m_pIndexPage->m_pIndexBuffer->buffer(), bc);
}

void ng::ResourceManager::mUploadToDevice(UniformBuffer& buffer, vk::CommandBuffer cb)   
{
	vk::BufferCopy bc{};
	bc.srcOffset = buffer.m_pStagingBuffer->m_pAllocation->getOffset();
	bc.dstOffset = buffer.m_pAllocation->getOffset();
	bc.size = buffer.m_Size;
	cb.copyBuffer(buffer.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->buffer(), buffer.m_pUniformPage->m_pUniformBuffer->buffer(), bc);
}

void ng::ResourceManager::mUploadToDevice(Texture2D& texture2D, vk::CommandBuffer cb)
{
	std::vector<vk::BufferImageCopy> bcRs;
	uint64 baseOffset = texture2D.m_pStagingBuffer->m_pAllocation->getOffset();
	for (int i = 0; i < texture2D.m_MipLevels; ++i) {
		ktx_size_t offset;
		KTX_error_code result = ktxTexture_GetImageOffset(texture2D.m_KTXTexture, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		vk::BufferImageCopy bcR{};
		bcR.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bcR.imageSubresource.mipLevel = i;
		bcR.imageSubresource.layerCount = 1;
		bcR.imageExtent.width = std::max(1u, texture2D.m_KTXTexture->baseWidth >> i);
		bcR.imageExtent.height = std::max(1u, texture2D.m_KTXTexture->baseHeight >> i);
		bcR.imageExtent.depth = 1;
		bcR.bufferOffset = baseOffset + offset;

		bcRs.push_back(bcR);
	}

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture2D.m_MipLevels;
	subresourceRange.layerCount = 1;

	ngv::setImageLayout(cb, texture2D.m_pVulkanTexture->image(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);

	cb.copyBufferToImage(texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->buffer(), texture2D.m_pVulkanTexture->image(),
		vk::ImageLayout::eTransferDstOptimal, static_cast<uint32>(bcRs.size()), bcRs.data());

	
	texture2D.m_pVulkanTexture->setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	ngv::setImageLayout(cb, texture2D.m_pVulkanTexture->image(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresourceRange);
}


void ng::ResourceManager::mGiveDeviceAllocation(VertexBuffer& vertexBuffer, vk::CommandBuffer cb)
{
	if (vertexBuffer.hasDeviceAllocation()) {
		return;
	}
	else {
		if (!vertexBuffer.hasStagingBuffer()) {
			mGiveStagingBuffer(vertexBuffer);
		}

		for (auto& page : m_BufferPages.vertexBufferPages) {
			bool ok = page.allocate(vertexBuffer);
			if (ok) {
				mUploadToDevice(vertexBuffer, cb);
				return;
			}
		}
#ifndef NDEBUG
		if (!mShouldUseNewDeviceVertexMemory()) {
			// TODO: free memory from other vertex allocations that arn't needed so that this one can allocate

			std::runtime_error("Couldn't find available memory for vertexBuffer and was not allowed to allocate more");
		}
#endif
		// We didn't find any page that could allocate, create a new one
		m_BufferPages.vertexBufferPages.emplace_back(*this);
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

		mUploadToDevice(vertexBuffer, cb);
		return;
	}

}

void ng::ResourceManager::mGiveDeviceAllocation(IndexBuffer& indexBuffer, vk::CommandBuffer cb)
{
	if (indexBuffer.hasDeviceAllocation()) {
		return;
	}
	else {
		if (!indexBuffer.hasStagingBuffer()) {
			mGiveStagingBuffer(indexBuffer);
		}

		for (auto& page : m_BufferPages.indexBufferPages) {
			bool ok = page.allocate(indexBuffer);
			if (ok) {
				return;
			}
		}
#ifndef NDEBUG
		if (!mShouldUseNewDeviceIndexMemory()) {
			// TODO: free memory from other index allocations that arn't needed so that this one can allocate

			std::runtime_error("Couldn't find available memory for vertexBuffer and was not allowed to allocate more");
		}
#endif
		// We didn't find any page that could allocate, create a new one
		m_BufferPages.indexBufferPages.emplace_back(*this);
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

		mUploadToDevice(indexBuffer, cb);
		return;
	}
}

void ng::ResourceManager::mGiveDeviceAllocation(UniformBuffer& uniformBuffer, vk::CommandBuffer cb)
{
	if (uniformBuffer.hasDeviceAllocation()) {
		return;
	}
	else {
		if (!uniformBuffer.hasStagingBuffer()) {
			mGiveStagingBuffer(uniformBuffer);
		}

		for (auto& page : m_BufferPages.uniformBufferPages) {
			bool ok = page.allocate(uniformBuffer);
			if (ok) {
				return;
			}
		}
#ifndef NDEBUG
		if (!mShouldUseNewDeviceUniformMemory()) {
			// TODO: free memory from other uniform allocations that arn't needed so that this one can allocate

			std::runtime_error("Couldn't find available memory for vertexBuffer and was not allowed to allocate more");
		}
#endif
		// We didn't find any page that could allocate, create a new one
		m_BufferPages.uniformBufferPages.emplace_back(*this);
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

		mUploadToDevice(uniformBuffer, cb);
		return;
	}
}

void ng::ResourceManager::mGiveDeviceAllocation(Texture2D& texture2D, vk::CommandBuffer cb)
{
	if (texture2D.hasDeviceAllocation()) {
		return;
	}
	else {
		if (!texture2D.hasStagingBuffer()) {
			mGiveStagingBuffer(texture2D);
		}

		using RD = ResourceResidencyFlagBits;
		if (!mShouldUseNewDeviceTexture2DMemory()) {
			// We must find some similar texture2D than is available
			// check first if there are some textures that require no residency
			auto map = m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency];
			for (auto& it : map) {
				if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height)
					&& (it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
				{
					//This one can be swapped
					texture2D.m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);

					map.erase(it.first);
					m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);

					mUploadToDevice(texture2D, cb);
					return;
				}
			}
			map = m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eStagingResidency];
			for (auto& it : map) { // This will be slow O(n) where n is number of images needing staging
				if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
					(it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
				{
					auto it2 = m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].find(it.first);
					if (it2 != m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].end()) {
						continue;
					}
					else { // We found a match
						texture2D.m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);

						map.erase(it.first);
						m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
						mUploadToDevice(texture2D, cb);
						return;
					}
				}
			}
			// IF we are here we are in big trouble
#ifndef NDEBUG
			std::runtime_error("Was not allowed to allocate more image memory for Texture2D and could not find any other swappable Texture2D");
#endif
		}

		texture2D.m_pVulkanTexture = ngv::VulkanTexture2D::make(m_Device,
			texture2D.m_Width, texture2D.m_Height, texture2D.m_MipLevels, texture2D.m_Format);
		m_Allocator.giveImageAllocation(*texture2D.m_pVulkanTexture);
		m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
		mUploadToDevice(texture2D, cb);
	}
}








bool ng::ResourceManager::mShouldUseNewStagingMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewDeviceVertexMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewHostVertexMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewDeviceIndexMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewHostIndexMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewDeviceUniformMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewHostUniformMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewDeviceTexture2DMemory()
{
	return true;
}

bool ng::ResourceManager::mShouldUseNewHostTexture2DMemory()
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

