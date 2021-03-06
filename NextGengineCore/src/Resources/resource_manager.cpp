#include "resource_manager.h"
#include "resources.h"


#include "../Memory/abstract_allocators.h"

#include "../Vulkan/vulkan_utility.h"
#include "../Vulkan/vulkan_device.h"
#include "../Vulkan/vulkan_allocator.h"
#include "../Vulkan/vulkan_storage.h"

// KTX
#include <ktx.h>
#include <ktxvulkan.h>

// <==================================== RESOURCE MANAGER ==========================================>

//PUBLIC

ng::ResourceManager::ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device, ResourceStrategy& strategy)
	: m_Allocator(allocator), m_Device(device), m_Strategy(strategy)
{
	// Setup Staging Buffer Pages
	{
		m_BufferPages.stagingBufferPages.emplace_back(std::move(std::unique_ptr<StagingBufferPage>(new StagingBufferPage(*this))));
		auto& page = m_BufferPages.stagingBufferPages.back();
		vk::BufferCreateInfo ci{};
		ci.size = m_Strategy.stagingBufferPageSize;
		ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
		ci.sharingMode = vk::SharingMode::eExclusive;
		page->m_pStagingBuffer = ngv::VulkanBuffer::Make(device, ci, true);
		allocator.GiveBufferAllocation(*page->m_pStagingBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::Make(ci.size);
	}
	// Setup Vertex Buffer Pages
	{
		m_BufferPages.vertexBufferPages.emplace_back(std::move(std::unique_ptr<VertexBufferPage>(new VertexBufferPage(*this))));
		auto& page = m_BufferPages.vertexBufferPages.back();
		page->m_pVertexBuffer = ngv::VulkanVertexBuffer::Make(device, strategy.hostVertexBufferPageSize, false);
		allocator.GiveBufferAllocation(*page->m_pVertexBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::Make(strategy.hostVertexBufferPageSize);
	}

	// Setup Index Buffer Pages
	{
		m_BufferPages.indexBufferPages.emplace_back(std::move(std::unique_ptr<IndexBufferPage>(new IndexBufferPage(*this))));
		auto& page = m_BufferPages.indexBufferPages.back();
		page->m_pIndexBuffer = ngv::VulkanIndexBuffer::Make(device, strategy.hostIndexBufferPageSize, false);
		allocator.GiveBufferAllocation(*page->m_pIndexBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::Make(strategy.hostIndexBufferPageSize);
	}

	// Setup Uniform Buffer Pages
	{
		m_BufferPages.uniformBufferPages.emplace_back(std::move(std::unique_ptr<UniformBufferPage>(new UniformBufferPage(*this))));
		auto& page = m_BufferPages.uniformBufferPages.back();
		page->m_pUniformBuffer = ngv::VulkanUniformBuffer::Make(device, strategy.hostUniformBufferPageSize, false);
		allocator.GiveBufferAllocation(*page->m_pUniformBuffer);
		page->m_pAllocator = AbstractFreeListAllocator::Make(strategy.hostIndexBufferPageSize);
	}

}

const ngv::VulkanDevice& ng::ResourceManager::vulkanDevice() const
{
	return m_Device;
}

std::shared_ptr<ng::VertexBuffer> ng::ResourceManager::getVertexBuffer(std::string& m_ID, std::function<std::vector<uint8>()> loadVertexBytes)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Buffers.vertexBuffersByID.find(m_ID);
	if (it != m_Buffers.vertexBuffersByID.end()) {
		return it->second.lock();
	}
	std::shared_ptr<VertexBuffer> ret = std::shared_ptr<VertexBuffer>(new VertexBuffer(*this, m_ID));
	m_Buffers.vertexBuffersByID.emplace(m_ID, ret);
	ret->m_LoadVertexBytes = loadVertexBytes;
	return ret;
}

std::shared_ptr<ng::IndexBuffer> ng::ResourceManager::getIndexBuffer(std::string& m_ID, std::function<std::vector<uint8>()> loadIndexBytes)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Buffers.indexBuffersByID.find(m_ID);
	if (it != m_Buffers.indexBuffersByID.end()) {
		return it->second.lock();
	}
	std::shared_ptr<IndexBuffer> ret = std::shared_ptr<IndexBuffer>(new IndexBuffer(*this, m_ID));
	m_Buffers.indexBuffersByID.emplace(m_ID, ret);
	ret->m_LoadIndexBytes = loadIndexBytes;
	return ret;
}

std::shared_ptr<ng::UniformBuffer> ng::ResourceManager::getUniformBuffer(std::string& m_ID, std::function<std::vector<uint8>()> loadUniformBytes)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Buffers.uniformBuffersByID.find(m_ID);
	if (it != m_Buffers.uniformBuffersByID.end()) {
		return it->second.lock();
	}
	std::shared_ptr<UniformBuffer> ret = std::shared_ptr<UniformBuffer>(new UniformBuffer(*this, m_ID));
	m_Buffers.uniformBuffersByID.emplace(m_ID, ret);
	ret->m_LoadUniformBytes = loadUniformBytes;
	return ret;
}

std::shared_ptr<ng::Texture2D> ng::ResourceManager::getTexture2D(std::string& m_ID)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Textures.texture2DsByID.find(m_ID);
	if (it != m_Textures.texture2DsByID.end()) {
		return it->second.lock();
	}
	std::shared_ptr<Texture2D> ret = std::shared_ptr<Texture2D>(new Texture2D(*this, m_ID));

	mGiveStagingBuffer(*ret);
	m_Textures.texture2DsByID.emplace(m_ID, ret);
	return ret;
}





void ng::ResourceManager::removeVertexBuffer(VertexBuffer& vertexBuffer)
{
	auto it = m_Buffers.vertexBuffersByID.find(vertexBuffer.m_ID);
	if (auto vb = it->second.lock()) {
		return;
	}
	else {
		m_Buffers.vertexBuffersByID.erase(vertexBuffer.m_ID);
		m_Buffers.vertexResidencyMaps[vertexBuffer.m_Residency][vertexBuffer.m_RequiredResidency].erase(vertexBuffer.m_ID);
	}
}

void ng::ResourceManager::removeIndexBuffer(IndexBuffer& indexBuffer)
{
	auto it = m_Buffers.indexBuffersByID.find(indexBuffer.m_ID);
	if (auto ib = it->second.lock()) {
		return;
	}
	else {
		m_Buffers.indexBuffersByID.erase(indexBuffer.m_ID);
		m_Buffers.indexResidencyMaps[indexBuffer.m_Residency][indexBuffer.m_RequiredResidency].erase(indexBuffer.m_ID);
	}
}

void ng::ResourceManager::removeUniformBuffer(UniformBuffer& uniformBuffer)
{
	auto it = m_Buffers.uniformBuffersByID.find(uniformBuffer.m_ID);
	if (auto ub = it->second.lock()) {
		return;
	}
	else {
		m_Buffers.uniformBuffersByID.erase(uniformBuffer.m_ID);
		m_Buffers.uniformResidencyMaps[uniformBuffer.m_Residency][uniformBuffer.m_RequiredResidency].erase(uniformBuffer.m_ID);
	}
}

void ng::ResourceManager::removeTexture2D(Texture2D& texture2D)
{
	auto it = m_Textures.texture2DsByID.find(texture2D.m_ID);
	if (auto tx = it->second.lock()) {
		return;
	}
	else {
		m_Textures.texture2DsByID.erase(texture2D.m_ID);
		m_Textures.texture2DResidencyMaps[texture2D.m_Residency][texture2D.m_RequiredResidency].erase(texture2D.m_ID);
	}
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



void ng::ResourceManager::setDeviceAllocationNotRequired(VertexBuffer& vertexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetDeviceAllocationNotRequired(vertexBuffer);
}

void ng::ResourceManager::setDeviceAllocationNotRequired(IndexBuffer& indexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetDeviceAllocationNotRequired(indexBuffer);
}

void ng::ResourceManager::setDeviceAllocationNotRequired(UniformBuffer& uniformBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetDeviceAllocationNotRequired(uniformBuffer);
}

void ng::ResourceManager::setDeviceAllocationNotRequired(Texture2D& texture2D)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetDeviceAllocationNotRequired(texture2D);
}

void ng::ResourceManager::setStagingBufferNotRequired(VertexBuffer& vertexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetStagingBufferNotRequired(vertexBuffer);
}

void ng::ResourceManager::setStagingBufferNotRequired(IndexBuffer& indexBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetStagingBufferNotRequired(indexBuffer);
}

void ng::ResourceManager::setStagingBufferNotRequired(UniformBuffer& uniformBuffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetStagingBufferNotRequired(uniformBuffer);
}

void ng::ResourceManager::setStagingBufferNotRequired(Texture2D& texture2D)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mSetStagingBufferNotRequired(texture2D);
}


// PRIVATE

//should always be used in conjunction with uploadToStagingBuffer, no resource should be able to have a staging buffer that doesn't hold it's stagingData
std::unique_ptr<ng::StagingBuffer> ng::ResourceManager::mGetStagingBuffer(std::string id, uint64 size)
{
	std::unique_ptr<StagingBuffer> ret = std::unique_ptr<StagingBuffer>(new StagingBuffer(*this, id, size));

	for (auto& page : m_BufferPages.stagingBufferPages) {
		bool ok = page->allocate(*ret);
		if (ok) {
			return ret;
		}
	}
#ifndef NDEBUG
	if (!mhShouldUseNewStagingMemory()) {
		// TODO: free memory from other staging allocations that arn't needed so that this one can allocate
		std::runtime_error("Couldn't find any available memory for stagingBuffer and was not allowed to allocate more");
	}
#endif

	m_BufferPages.stagingBufferPages.emplace_back(std::move(std::unique_ptr<StagingBufferPage>(new StagingBufferPage(*this))));
	auto& page = m_BufferPages.stagingBufferPages.back();

	uint64 pageSize = m_Strategy.stagingBufferPageSize;
	if (ret->m_Size > pageSize) {
		pageSize = ret->m_Size;
	}

	vk::BufferCreateInfo ci{};
	ci.size = pageSize;
	ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
	ci.sharingMode = vk::SharingMode::eExclusive;
	page->m_pStagingBuffer = ngv::VulkanBuffer::Make(m_Device, ci, true);
	page->m_pAllocator = AbstractFreeListAllocator::Make(pageSize);

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


void ng::ResourceManager::mGiveStagingBuffer(VertexBuffer& vertexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if (vertexBuffer.hasStagingBuffer()) {
		if ((vertexBuffer.m_RequiredResidency) == (uint8)RD::eStagingResidency || (vertexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (vertexBuffer.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
			m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(vertexBuffer.m_ID);
			vertexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else {
			m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
			vertexBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
		}
	}
	else {
		auto bytes = vertexBuffer.m_LoadVertexBytes();
		vertexBuffer.m_Size = bytes.size();
		vertexBuffer.m_pStagingBuffer = mGetStagingBuffer(vertexBuffer.m_ID, vertexBuffer.m_Size);
		mhUploadToStagingBuffer(*vertexBuffer.m_pStagingBuffer, bytes.data());
		
		
		m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
		if (vertexBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) {
			if (vertexBuffer.m_Residency == (uint8)RD::eDeviceResidency) {
				m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(vertexBuffer.m_ID);
				vertexBuffer.m_Residency = (uint8)RD::eAllResidency;
			}
			vertexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else if (vertexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
			vertexBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
			vertexBuffer.m_Residency = (uint8)RD::eAllResidency; // if it required device residency it has it
		}
		else {
			// if we are here it can't have had device residency
			vertexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
			vertexBuffer.m_Residency = (uint8)RD::eStagingResidency;
		}
	}
}

void ng::ResourceManager::mGiveStagingBuffer(IndexBuffer& indexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if (indexBuffer.hasStagingBuffer()) {
		if ((indexBuffer.m_RequiredResidency) == (uint8)RD::eStagingResidency || (indexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (indexBuffer.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(indexBuffer.m_ID, &indexBuffer);
			m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(indexBuffer.m_ID);
			indexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else {
			m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(indexBuffer.m_ID, &indexBuffer);
			indexBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
		}
	}
	else {
		auto bytes = indexBuffer.m_LoadIndexBytes();
		indexBuffer.m_Size = bytes.size();
		indexBuffer.m_pStagingBuffer = mGetStagingBuffer(indexBuffer.m_ID, indexBuffer.m_Size);
		mhUploadToStagingBuffer(*indexBuffer.m_pStagingBuffer, bytes.data());

		m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(indexBuffer.m_ID, &indexBuffer);
		if (indexBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) {
			if (indexBuffer.m_Residency == (uint8)RD::eDeviceResidency) {
				m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(indexBuffer.m_ID);
				indexBuffer.m_Residency = (uint8)RD::eAllResidency;
			}
			indexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else if (indexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
			indexBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
			indexBuffer.m_Residency = (uint8)RD::eAllResidency; // if it required device residency it has it
		}
		else {
			// if we are here it can't have had device residency
			indexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
			indexBuffer.m_Residency = (uint8)RD::eStagingResidency;
		}

	}
}

void ng::ResourceManager::mGiveStagingBuffer(UniformBuffer& uniformBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if (uniformBuffer.hasStagingBuffer()) {
		if ((uniformBuffer.m_RequiredResidency) == (uint8)RD::eStagingResidency || (uniformBuffer.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (uniformBuffer.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
			m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(uniformBuffer.m_ID);
			uniformBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else {
			m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
			uniformBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
		}
	}
	else {
		auto bytes = uniformBuffer.m_LoadUniformBytes();
		uniformBuffer.m_Size = bytes.size();
		uniformBuffer.m_pStagingBuffer = mGetStagingBuffer(uniformBuffer.m_ID, uniformBuffer.m_Size);
		mhUploadToStagingBuffer(*uniformBuffer.m_pStagingBuffer, bytes.data());

		m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
		if (uniformBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) {
			if (uniformBuffer.m_Residency == (uint8)RD::eDeviceResidency) {
				m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(uniformBuffer.m_ID);
				uniformBuffer.m_Residency = (uint8)RD::eAllResidency;
			}
			uniformBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else if (uniformBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
			uniformBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
			uniformBuffer.m_Residency = (uint8)RD::eAllResidency; // if it required device residency it has it
		}
		else {
			// if we are here it can't have had device residency
			uniformBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
			uniformBuffer.m_Residency = (uint8)RD::eStagingResidency;
		}

	}
}

void ng::ResourceManager::mGiveStagingBuffer(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	if (texture2D.hasStagingBuffer()) {
		if ((texture2D.m_RequiredResidency) == (uint8)RD::eStagingResidency || (texture2D.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (texture2D.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
			m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(texture2D.m_ID);
			texture2D.m_RequiredResidency = (uint8)RD::eStagingResidency;
		}
		else {
			m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
			texture2D.m_RequiredResidency = (uint8)RD::eAllResidency;
		}
	}
	else {
		if (mhGiveStaging(texture2D)) {
			// continue to final:
		}
		else if (!mhShouldUseNewStagingMemory()) {
			mhGiveStagingBySwapping(texture2D);
			// continue to final:
		}
		else { // We can use new staging memory so make a new page and allocate from it
			m_BufferPages.stagingBufferPages.emplace_back(std::move(std::unique_ptr<StagingBufferPage>(new StagingBufferPage(*this))));
			auto& page = m_BufferPages.stagingBufferPages.back();

			uint64 pageSize = m_Strategy.stagingBufferPageSize;
			if (texture2D.m_pStagingBuffer->m_Size > pageSize) {
				pageSize = texture2D.m_pStagingBuffer->m_Size;
			}

			vk::BufferCreateInfo ci{};
			ci.size = pageSize;
			ci.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;
			ci.sharingMode = vk::SharingMode::eExclusive;
			page->m_pStagingBuffer = ngv::VulkanBuffer::Make(m_Device, ci, true);
			page->m_pAllocator = ng::AbstractFreeListAllocator::Make(pageSize);

#ifndef NDEBUG
			bool ok = page->allocate(*texture2D.m_pStagingBuffer);
			if (!ok) {
				std::runtime_error("Created new page that could not allocate");
			}
#else
			page->allocate(*texture2D.m_pStagingBuffer);
#endif
			m_UsedHostMemory += pageSize;

			// continue to final:
		}
	}

	// We will only arrive here if there wern't already a staging buffer
final:
	mhUploadToStagingBuffer(*texture2D.m_pStagingBuffer, (uint8*)ktxTexture_GetData(texture2D.m_pKTXTexture));

	m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].emplace(texture2D.m_ID, &texture2D);
	if (texture2D.m_RequiredResidency == (uint8)RD::eNoResidency) {
		if (texture2D.m_Residency == (uint8)RD::eDeviceResidency) {
			m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(texture2D.m_ID);
			texture2D.m_Residency = (uint8)RD::eAllResidency;
		}
		texture2D.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (texture2D.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
		texture2D.m_RequiredResidency = (uint8)RD::eAllResidency;
		texture2D.m_Residency = (uint8)RD::eAllResidency; // if it required device residency it has it
	}
	else {
		// if we are here it can't have had device residency
		texture2D.m_RequiredResidency = (uint8)RD::eStagingResidency;
		texture2D.m_Residency = (uint8)RD::eStagingResidency;
	}
}



void ng::ResourceManager::mGiveDeviceAllocation(VertexBuffer& vertexBuffer, vk::CommandBuffer cb)
{
	using RD = ResourceResidencyFlagBits;
	if (vertexBuffer.hasDeviceAllocation()) {
		if ((vertexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) || (vertexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (vertexBuffer.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
			m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(vertexBuffer.m_ID);
			vertexBuffer.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		else {
			m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
			vertexBuffer.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		return;
	}
	else {
		if (!vertexBuffer.hasStagingBuffer()) {
			mGiveStagingBuffer(vertexBuffer);
		}

		for (auto& page : m_BufferPages.vertexBufferPages) {
			bool ok = page->allocate(vertexBuffer);
			if (ok) {
				goto final;
			}
		}

		if (!mhShouldUseNewDeviceVertexMemory()) {
			// TODO: free memory from other vertex allocations that arn't needed so that this one can allocate
			//continue to final
		}
		else {
			// We didn't find any page that could allocate, create a new one
			m_BufferPages.vertexBufferPages.emplace_back(std::move(std::unique_ptr<VertexBufferPage>(new VertexBufferPage(*this))));
			auto page = &m_BufferPages.vertexBufferPages.back();

			uint64 pageSize = m_Strategy.deviceVertexBufferPageSize;
			if (vertexBuffer.m_Size > pageSize) {
				pageSize = vertexBuffer.m_Size;
			}

			page->get()->m_pVertexBuffer = ngv::VulkanVertexBuffer::Make(m_Device, pageSize, false);
			page->get()->m_pAllocator = AbstractFreeListAllocator::Make(pageSize);

#ifndef NDEBUG
			bool ok = page->get()->allocate(vertexBuffer);
			if (!ok) {
				std::runtime_error("Created new page that could not allocate");
			}
#else
			page->allocate(vertexBuffer);
#endif
			m_UsedDeviceMemory += pageSize;

			//continue to final
		}
	}


final:
	mhUploadToDevice(vertexBuffer, cb);
	m_Buffers.vertexResidencyMaps[(uint8)RD::eDeviceResidency][(uint8)RD::eDeviceResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
	vertexBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
	vertexBuffer.m_Residency = (uint8)RD::eAllResidency;
	return;
}

void ng::ResourceManager::mGiveDeviceAllocation(IndexBuffer& indexBuffer, vk::CommandBuffer cb)
{
	using RD = ResourceResidencyFlagBits;
	if (indexBuffer.hasDeviceAllocation()) {
		if ((indexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) || (indexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (indexBuffer.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(indexBuffer.m_ID, &indexBuffer);
			m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(indexBuffer.m_ID);
			indexBuffer.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		else {
			m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(indexBuffer.m_ID, &indexBuffer);
			indexBuffer.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		return;
	}
	else {
		if (!indexBuffer.hasStagingBuffer()) {
			mGiveStagingBuffer(indexBuffer);
		}

		for (auto& page : m_BufferPages.indexBufferPages) {
			bool ok = page->allocate(indexBuffer);
			if (ok) {
				goto final;
			}
		}

		if (!mhShouldUseNewDeviceIndexMemory()) {
			// TODO: free memory from other index allocations that arn't needed so that this one can allocate
			//continue to final
		}
		else {
			// We didn't find any page that could allocate, create a new one
			m_BufferPages.indexBufferPages.emplace_back(std::move(std::unique_ptr<IndexBufferPage>(new IndexBufferPage(*this))));
			auto& page = m_BufferPages.indexBufferPages.back();

			uint64 pageSize = m_Strategy.deviceIndexBufferPageSize;
			if (indexBuffer.m_Size > pageSize) {
				pageSize = indexBuffer.m_Size;
			}

			page->m_pIndexBuffer = ngv::VulkanIndexBuffer::Make(m_Device, pageSize, false);
			page->m_pAllocator = AbstractFreeListAllocator::Make(pageSize);

#ifndef NDEBUG
			bool ok = page->allocate(indexBuffer);
			if (!ok) {
				std::runtime_error("Created new page that could not allocate");
			}
#else
			page->allocate(indexBuffer);
#endif
			m_UsedDeviceMemory += pageSize;

			//continue to final
		}
	}

final:
	mhUploadToDevice(indexBuffer, cb);
	m_Buffers.indexResidencyMaps[(uint8)RD::eDeviceResidency][(uint8)RD::eDeviceResidency].emplace(indexBuffer.m_ID, &indexBuffer);
	indexBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
	indexBuffer.m_Residency = (uint8)RD::eAllResidency;
	return;
}

void ng::ResourceManager::mGiveDeviceAllocation(UniformBuffer& uniformBuffer, vk::CommandBuffer cb)
{
	using RD = ResourceResidencyFlagBits;
	if (uniformBuffer.hasDeviceAllocation()) {
		if ((uniformBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) || (uniformBuffer.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (uniformBuffer.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
			m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(uniformBuffer.m_ID);
			uniformBuffer.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		else {
			m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
			uniformBuffer.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		return;
	}
	else {
		if (!uniformBuffer.hasStagingBuffer()) {
			mGiveStagingBuffer(uniformBuffer);
		}

		for (auto& page : m_BufferPages.uniformBufferPages) {
			bool ok = page->allocate(uniformBuffer);
			if (ok) {
				goto final;
			}
		}

		if (!mhShouldUseNewDeviceUniformMemory()) {
			// TODO: free memory from other uniform allocations that arn't needed so that this one can allocate
			//continue to final
		}
		else {
			// We didn't find any page that could allocate, create a new one
			m_BufferPages.uniformBufferPages.emplace_back(std::move(std::unique_ptr<UniformBufferPage>(new UniformBufferPage(*this))));
			auto& page = m_BufferPages.uniformBufferPages.back();

			uint64 pageSize = m_Strategy.deviceUniformBufferPageSize;
			if (uniformBuffer.m_Size > pageSize) {
				pageSize = uniformBuffer.m_Size;
			}

			page->m_pUniformBuffer = ngv::VulkanUniformBuffer::Make(m_Device, pageSize, false);
			page->m_pAllocator = AbstractFreeListAllocator::Make(pageSize);

#ifndef NDEBUG
			bool ok = page->allocate(uniformBuffer);
			if (!ok) {
				std::runtime_error("Created new page that could not allocate");
			}
#else
			page->allocate(uniformBuffer);
#endif
			m_UsedDeviceMemory += pageSize;

			// continue to final
		}
	}

final:
	mhUploadToDevice(uniformBuffer, cb);
	m_Buffers.uniformResidencyMaps[(uint8)RD::eDeviceResidency][(uint8)RD::eDeviceResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
	uniformBuffer.m_RequiredResidency = (uint8)RD::eAllResidency;
	uniformBuffer.m_Residency = (uint8)RD::eAllResidency;
	return;
}

void ng::ResourceManager::mGiveDeviceAllocation(Texture2D& texture2D, vk::CommandBuffer cb)

{
	using RD = ResourceResidencyFlagBits;

	if (texture2D.hasDeviceAllocation()) {
		if ((texture2D.m_RequiredResidency == (uint8)RD::eDeviceResidency) || (texture2D.m_RequiredResidency == (uint8)RD::eAllResidency)) {
			// Do nothing
		}
		else if (texture2D.m_RequiredResidency == (int)RD::eNoResidency) {
			m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
			m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(texture2D.m_ID);
			texture2D.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		else {
			m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
			texture2D.m_RequiredResidency = (uint8)RD::eDeviceResidency;
		}
		return;
	}
	else {
		// If there are no staging buffer we need to give it one so we can upload the data
		if (!texture2D.hasStagingBuffer()) {
			mGiveStagingBuffer(texture2D);
		}

		// If we shouldn't use more memory we must swap some textures
		if (!mhShouldUseNewDeviceTexture2DMemory()) {
			mhGiveDeviceBySwapping(texture2D, cb);
			//continue to final
		}
		else {
			// We were allowed to use more memory so let's create a new vulkan texture
			texture2D.m_pVulkanTexture = ngv::VulkanTexture2D::Make(m_Device,
				texture2D.m_Width, texture2D.m_Height, texture2D.m_MipLevels, texture2D.m_Format);
			m_Allocator.GiveImageAllocation(*texture2D.m_pVulkanTexture);
			//continue to final
		}
	}

final:
	mhUploadToDevice(texture2D, cb);
	m_Textures.texture2DResidencyMaps[(uint8)RD::eDeviceResidency][(uint8)RD::eDeviceResidency].emplace(texture2D.m_ID, &texture2D);
	texture2D.m_RequiredResidency = (uint8)RD::eAllResidency;
	texture2D.m_Residency = (uint8)RD::eAllResidency;
	return;
}





void ng::ResourceManager::mSetDeviceAllocationNotRequired(VertexBuffer& vertexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if ((vertexBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) || (vertexBuffer.m_RequiredResidency == (uint8)RD::eStagingResidency)) {
		return;
	}
	else if (vertexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency) {
		m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(vertexBuffer.m_ID);
		vertexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (vertexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
		m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(vertexBuffer.m_ID);
		m_Buffers.vertexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
		vertexBuffer.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

void ng::ResourceManager::mSetDeviceAllocationNotRequired(IndexBuffer& indexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if ((indexBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) || (indexBuffer.m_RequiredResidency == (uint8)RD::eStagingResidency)) {
		return;
	}
	else if (indexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency) {
		m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(indexBuffer.m_ID);
		indexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (indexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
		m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(indexBuffer.m_ID);
		m_Buffers.indexResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].emplace(indexBuffer.m_ID, &indexBuffer);
		indexBuffer.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

void ng::ResourceManager::mSetDeviceAllocationNotRequired(UniformBuffer& uniformBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if ((uniformBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) || (uniformBuffer.m_RequiredResidency == (uint8)RD::eStagingResidency)) {
		return;
	}
	else if (uniformBuffer.m_RequiredResidency == (uint8)RD::eAllResidency) {
		m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(uniformBuffer.m_ID);
		uniformBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (uniformBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency) {
		m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(uniformBuffer.m_ID);
		m_Buffers.uniformResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
		uniformBuffer.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

void ng::ResourceManager::mSetDeviceAllocationNotRequired(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	if ((texture2D.m_RequiredResidency == (uint8)RD::eNoResidency) || (texture2D.m_RequiredResidency == (uint8)RD::eStagingResidency)) {
		return;
	}
	else if (texture2D.m_RequiredResidency == (int)RD::eAllResidency) {
		m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(texture2D.m_ID);
		texture2D.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (texture2D.m_RequiredResidency == (int)RD::eDeviceResidency) {
		m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eDeviceResidency].erase(texture2D.m_ID);
		m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].emplace(texture2D.m_ID, &texture2D);
		texture2D.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

						  
						  
void ng::ResourceManager::mSetStagingBufferNotRequired(VertexBuffer& vertexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if ((vertexBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) || (vertexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency)) {
		return;
	}
	else if (vertexBuffer.m_RequiredResidency == (int)RD::eAllResidency) {
		m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(vertexBuffer.m_ID);
		vertexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (vertexBuffer.m_RequiredResidency == (uint8)RD::eStagingResidency) {
		m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(vertexBuffer.m_ID);
		m_Buffers.vertexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].emplace(vertexBuffer.m_ID, &vertexBuffer);
		vertexBuffer.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

void ng::ResourceManager::mSetStagingBufferNotRequired(IndexBuffer& indexBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if ((indexBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) || (indexBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency)) {
		return;
	}
	else if (indexBuffer.m_RequiredResidency == (uint8)RD::eAllResidency) {
		m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(indexBuffer.m_ID);
		indexBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (indexBuffer.m_RequiredResidency == (uint8)RD::eStagingResidency) {
		m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(indexBuffer.m_ID);
		m_Buffers.indexResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].emplace(indexBuffer.m_ID, &indexBuffer);
		indexBuffer.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

void ng::ResourceManager::mSetStagingBufferNotRequired(UniformBuffer& uniformBuffer)
{
	using RD = ResourceResidencyFlagBits;
	if ((uniformBuffer.m_RequiredResidency == (uint8)RD::eNoResidency) || (uniformBuffer.m_RequiredResidency == (uint8)RD::eDeviceResidency)) {
		return;
	}
	else if (uniformBuffer.m_RequiredResidency == (uint8)RD::eAllResidency) {
		m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(uniformBuffer.m_ID);
		uniformBuffer.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (uniformBuffer.m_RequiredResidency == (uint8)RD::eStagingResidency) {
		m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(uniformBuffer.m_ID);
		m_Buffers.uniformResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].emplace(uniformBuffer.m_ID, &uniformBuffer);
		uniformBuffer.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}

void ng::ResourceManager::mSetStagingBufferNotRequired(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	if ((texture2D.m_RequiredResidency == (uint8)RD::eNoResidency) || (texture2D.m_RequiredResidency == (uint8)RD::eDeviceResidency)) {
		return;
	}
	else if (texture2D.m_RequiredResidency == (uint8)RD::eAllResidency) {
		m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(texture2D.m_ID);
		texture2D.m_RequiredResidency = (uint8)RD::eStagingResidency;
	}
	else if (texture2D.m_RequiredResidency == (uint8)RD::eStagingResidency) {
		m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eStagingResidency].erase(texture2D.m_ID);
		m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].emplace(texture2D.m_ID, &texture2D);
		texture2D.m_RequiredResidency = (uint8)RD::eNoResidency;
	}
}




// Private helpers

void ng::ResourceManager::mhUploadToStagingBuffer(StagingBuffer& buffer, uint8* data)
{
	void* mapped = buffer.m_pStagingPage->m_pStagingBuffer->Map(
		buffer.m_pAllocation->GetOffset(), buffer.m_Size);
	memcpy(mapped, data, buffer.m_Size);
	buffer.m_pStagingPage->m_pStagingBuffer->Unmap();
}

void ng::ResourceManager::mhUploadToDevice(VertexBuffer& buffer, vk::CommandBuffer cb)
{
	vk::BufferCopy bc{};
	bc.srcOffset = buffer.m_pStagingBuffer->m_pAllocation->GetOffset();
	bc.dstOffset = buffer.m_pAllocation->GetOffset();
	bc.size = buffer.m_Size;
	cb.copyBuffer(buffer.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->Buffer(), buffer.m_pVertexPage->m_pVertexBuffer->Buffer(), bc);
}

void ng::ResourceManager::mhUploadToDevice(IndexBuffer& buffer, vk::CommandBuffer cb)
{
	vk::BufferCopy bc{};
	bc.srcOffset = buffer.m_pStagingBuffer->m_pAllocation->GetOffset();
	bc.dstOffset = buffer.m_pAllocation->GetOffset();
	bc.size = buffer.m_Size;
	cb.copyBuffer(buffer.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->Buffer(), buffer.m_pIndexPage->m_pIndexBuffer->Buffer(), bc);
}

void ng::ResourceManager::mhUploadToDevice(UniformBuffer& buffer, vk::CommandBuffer cb)
{
	vk::BufferCopy bc{};
	bc.srcOffset = buffer.m_pStagingBuffer->m_pAllocation->GetOffset();
	bc.dstOffset = buffer.m_pAllocation->GetOffset();
	bc.size = buffer.m_Size;
	cb.copyBuffer(buffer.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->Buffer(), buffer.m_pUniformPage->m_pUniformBuffer->Buffer(), bc);
}

void ng::ResourceManager::mhUploadToDevice(Texture2D& texture2D, vk::CommandBuffer cb)
{
	std::vector<vk::BufferImageCopy> bcRs;
	uint64 baseOffset = texture2D.m_pStagingBuffer->m_pAllocation->GetOffset();
	for (int i = 0; i < texture2D.m_MipLevels; ++i) {
		ktx_size_t offset;
		KTX_error_code result = ktxTexture_GetImageOffset(texture2D.m_pKTXTexture, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		vk::BufferImageCopy bcR{};
		bcR.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		bcR.imageSubresource.mipLevel = i;
		bcR.imageSubresource.layerCount = 1;
		bcR.imageExtent.width = std::max(1u, texture2D.m_pKTXTexture->baseWidth >> i);
		bcR.imageExtent.height = std::max(1u, texture2D.m_pKTXTexture->baseHeight >> i);
		bcR.imageExtent.depth = 1;
		bcR.bufferOffset = baseOffset + offset;

		bcRs.push_back(bcR);
	}

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture2D.m_MipLevels;
	subresourceRange.layerCount = 1;

	ngv::setImageLayout(cb, texture2D.m_pVulkanTexture->Image(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);

	cb.copyBufferToImage(texture2D.m_pStagingBuffer->m_pStagingPage->m_pStagingBuffer->Buffer(), texture2D.m_pVulkanTexture->Image(),
		vk::ImageLayout::eTransferDstOptimal, static_cast<uint32>(bcRs.size()), bcRs.data());


	texture2D.m_pVulkanTexture->SetImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	ngv::setImageLayout(cb, texture2D.m_pVulkanTexture->Image(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, subresourceRange);
}


/* Initializes a staging buffer for the texture and allocates it, notice no residency requirements of uploading is done here*/
bool ng::ResourceManager::mhGiveStaging(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	ktxResult result;
	result = ktxTexture_CreateFromNamedFile(texture2D.m_ID.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture2D.m_pKTXTexture);
	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(texture2D.m_pKTXTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(texture2D.m_pKTXTexture);
	texture2D.m_Width = texture2D.m_pKTXTexture->baseWidth;
	texture2D.m_Height = texture2D.m_pKTXTexture->baseHeight;
	texture2D.m_MipLevels = texture2D.m_pKTXTexture->numLevels;
	texture2D.m_Format = (vk::Format)ktxTexture_GetVkFormat(texture2D.m_pKTXTexture);
#ifndef NDEBUG
	if (texture2D.m_Format == vk::Format::eUndefined) {
		std::runtime_error("Texture had undefined format");
	}
#endif
	texture2D.m_pStagingBuffer = std::unique_ptr<StagingBuffer>(new StagingBuffer(*this, texture2D.m_ID, ktxTextureSize));

	// Basically reinvents mGetStagingBuffer
	for (auto& page : m_BufferPages.stagingBufferPages) {
		bool ok = page->allocate(*texture2D.m_pStagingBuffer);
		if (ok) {
			return true;
		}
	}

	return false;
}

/* Give this texture a staging buffer by swapping it with another texture, notice no residency requirements of uploading is done here*/
bool ng::ResourceManager::mhGiveStagingBySwapping(Texture2D& texture2D)
{
	using RD = ResourceResidencyFlagBits;
	// We must find some similar texture2D than is available
	// check first if there are some textures that require no residency
	for (auto& it : m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency]) {
		if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
			(it.second->m_Height == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
		{
			// Swap staging buffer
			texture2D.m_pStagingBuffer = std::move(it.second->m_pStagingBuffer);
			texture2D.m_pStagingBuffer->m_ID = texture2D.m_ID;
			
			// update the swapped texture
			// Swappable residency, (we don't change it's required residency)
			if (it.second->m_Residency == (uint8)RD::eAllResidency) {
				it.second->m_Residency = (uint8)RD::eDeviceResidency;
			}
			else {
				it.second->m_Residency = (uint8)RD::eNoResidency;
			}
			m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eNoResidency].erase(it.first);


			return true;
		}
	}
	for (auto& it : m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eDeviceResidency]) {
		if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
			(it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
		{
			if (it.second->m_RequiredResidency != (uint8)RD::eAllResidency) {
				// it only requires device residency so we can swap it
				// Swap staging buffer
				texture2D.m_pStagingBuffer = std::move(it.second->m_pStagingBuffer);
				texture2D.m_pStagingBuffer->m_ID = texture2D.m_ID;
				
				// update the swapped texture
				it.second->m_Residency = (uint8)RD::eDeviceResidency;
				m_Textures.texture2DResidencyMaps[(int)RD::eStagingResidency][(int)RD::eDeviceResidency].erase(it.first);

				return true;
			}
		}
	}
	return false;
}

/* Give this texture a vulkan-texture by swapping it with another texture, notice no residency requirements of uploading is done here*/
bool ng::ResourceManager::mhGiveDeviceBySwapping(Texture2D& texture2D, vk::CommandBuffer cb)
{
	using RD = ResourceResidencyFlagBits;
	// We must find some similar texture2D than is available
	// check first if there are some textures that require no residency
	for (auto& it : m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency]) {
		if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height)
			&& (it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
		{
			//This one can be swapped
			texture2D.m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);

			// The swappable textures residency (we don't change it's required residency)
			if (it.second->m_Residency == (uint8)RD::eAllResidency) {
				it.second->m_Residency = (uint8)RD::eStagingResidency;
			}
			else {
				it.second->m_Residency = (uint8)RD::eNoResidency;
			}
			m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eNoResidency].erase(it.first);

			return true;
		}
	}
	for (auto& it : m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eStagingResidency]) { // This will be slow O(n) where n is number of images needing staging
		if ((it.second->m_Format == texture2D.m_Format) && (it.second->m_Height == texture2D.m_Height) &&
			(it.second->m_Width == texture2D.m_Width) && (it.second->m_MipLevels == texture2D.m_MipLevels))
		{
			if (it.second->m_RequiredResidency != (uint8)RD::eAllResidency) {
				texture2D.m_pVulkanTexture = std::move(it.second->m_pVulkanTexture);

				// Swappable textures residency ( we don't need to update required residency)
				it.second->m_Residency = (uint8)RD::eStagingResidency;
				m_Textures.texture2DResidencyMaps[(int)RD::eDeviceResidency][(int)RD::eStagingResidency].erase(it.first);

				return true;
			}
		}
	}
	return false;
}

















































































bool ng::ResourceManager::mhShouldUseNewStagingMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewDeviceVertexMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewHostVertexMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewDeviceIndexMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewHostIndexMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewDeviceUniformMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewHostUniformMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewDeviceTexture2DMemory()
{
	return true;
}

bool ng::ResourceManager::mhShouldUseNewHostTexture2DMemory()
{
	return true;
}


















// PAGES
// Staging
bool ng::StagingBufferPage::allocate(StagingBuffer& stagingBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	stagingBuffer.m_pAllocation = m_pAllocator->Allocate(stagingBuffer.m_Size, 1);
	if (stagingBuffer.m_pAllocation == nullptr) {
		return false;
	}
	stagingBuffer.m_pStagingPage = ng::raw_ptr<StagingBufferPage>(this);
	return true;
}

void ng::StagingBufferPage::free(StagingBuffer& stagingBuffer)
{
	//std::lock_guard<std::mutex> lock(m_Mutex);
	m_pAllocator->Free(std::move(stagingBuffer.m_pAllocation));
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

