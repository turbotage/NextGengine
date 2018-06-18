#include "../vulkan_device.h"
#include "vulkan_buffer_region_allocator.h"
#include "../Threading/vulkan_synchronisation.h"

//VkBufferRegionAllocator

ng::vulkan::VulkanBufferRegionAllocator::VulkanBufferRegionAllocator(VulkanBufferRegionAllocatorCreateInfo createInfo)
{
	m_VulkanDevice = createInfo.vulkanDevice;
	m_VkBufferMemory = createInfo.vkBufferMemory;
	m_VkBuffer = createInfo.vkBuffer;
	m_MemorySize = createInfo.memorySize;
	m_MemoryAlignment = createInfo.memoryAlignment;

	m_FreeMemorySize = m_MemorySize;
}

uint32 ng::vulkan::VulkanBufferRegionAllocator::increaseBufferCopies(VulkanBuffer * buffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_Buffers.increaseBufferCopies(buffer);
}

std::pair<VkDeviceSize, VkDeviceSize> ng::vulkan::VulkanBufferRegionAllocator::findSuitableFreeSpace(VkDeviceSize size)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_FreeSpaces.findSuitableFreeSpace(size);
}

uint32 ng::vulkan::VulkanBufferRegionAllocator::getFreeSpaceCount()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_FreeSpaces.size();
}

bool ng::vulkan::VulkanBufferRegionAllocator::isInBufferRegion(VulkanBuffer* buffer)
{
	if (buffer->m_VkBuffer == this->m_VkBuffer) {
		return true;
	}
	/*
	auto bufferIt = Buffers.find(buffer);
	if (bufferIt != Buffers.end()) {
		if (bufferIt->second.buffer == *buffer) {
			return true;
		}
	}
	*/
	return false;
}

VkResult ng::vulkan::VulkanBufferRegionAllocator::createBuffer(VulkanBuffer* vulkanBuffer, VulkanBufferCreateInfo createInfo)
{

	VkDeviceSize allocSize = createInfo.size + m_MemoryAlignment -(createInfo.size % m_MemoryAlignment); //fix size to right mem-alignment

	std::lock_guard<std::mutex> lock(m_Mutex);

	auto freeSpaceRet = m_FreeSpaces.findSuitableFreeSpace(allocSize);

	VulkanBuffer::VulkanBufferInternalCreateInfo internalCreateInfo;
	internalCreateInfo.bufferRegionAllocator = this;
	internalCreateInfo.offset = freeSpaceRet.first;
	internalCreateInfo.size = allocSize;
	internalCreateInfo.vkBuffer = m_VkBuffer;
	internalCreateInfo.data = createInfo.data;
	internalCreateInfo.dataSize = createInfo.dataSize;
	internalCreateInfo.onUpdateCallback = createInfo.onUpdateCallback;

	auto it = m_Buffers.emplace(internalCreateInfo);
	it.first->second.copiedBuffers.push_back(vulkanBuffer);

	//set the values of the calling buffer
	vulkanBuffer->m_BufferRegionAllocator = this;
	vulkanBuffer->m_Offset = internalCreateInfo.offset;
	vulkanBuffer->m_Size = internalCreateInfo.size;
	vulkanBuffer->m_VkBuffer = m_VkBuffer;
	vulkanBuffer->m_Data = internalCreateInfo.data;
	vulkanBuffer->m_DataSize = internalCreateInfo.dataSize;
	vulkanBuffer->m_OnUpdateCallback = internalCreateInfo.onUpdateCallback;

	m_FreeSpaces.erase(freeSpaceRet.first, freeSpaceRet.second); //remove the old freespace
	m_FreeSpaces.insert(internalCreateInfo.offset + internalCreateInfo.size, freeSpaceRet.second - internalCreateInfo.size); //insert the new freespace
	m_FreeMemorySize -= allocSize;

	if (createInfo.data != nullptr) {
		return write(vulkanBuffer, createInfo.data, createInfo.dataSize);
	}
	return VK_SUCCESS;
}

VkResult ng::vulkan::VulkanBufferRegionAllocator::createBuffer(VulkanBuffer* vulkanBuffer, VkDeviceSize freeSpaceOffset, VkDeviceSize freeSpaceSize, VulkanBufferCreateInfo createInfo)
{

	VulkanBuffer::VulkanBufferInternalCreateInfo internalCreateInfo;
	internalCreateInfo.bufferRegionAllocator = this;
	internalCreateInfo.offset = freeSpaceOffset;
	internalCreateInfo.size = createInfo.size;
	internalCreateInfo.vkBuffer = m_VkBuffer;
	internalCreateInfo.data = createInfo.data;
	internalCreateInfo.dataSize = createInfo.dataSize;
	internalCreateInfo.onUpdateCallback = createInfo.onUpdateCallback;

	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Buffers.emplace(internalCreateInfo);
	it.first->second.copiedBuffers.push_back(vulkanBuffer);
	
	//set the values of the calling buffer
	vulkanBuffer->m_BufferRegionAllocator = this;
	vulkanBuffer->m_Offset = internalCreateInfo.offset;
	vulkanBuffer->m_Size = internalCreateInfo.size;
	vulkanBuffer->m_VkBuffer = m_VkBuffer;
	vulkanBuffer->m_Data = internalCreateInfo.data;
	vulkanBuffer->m_DataSize = internalCreateInfo.dataSize;
	vulkanBuffer->m_OnUpdateCallback = internalCreateInfo.onUpdateCallback;

	m_FreeSpaces.erase(freeSpaceOffset, freeSpaceSize);
	m_FreeSpaces.insert(freeSpaceOffset + createInfo.size, freeSpaceSize - createInfo.size);
	m_FreeMemorySize -= createInfo.size;

	if (createInfo.data != nullptr) {
		return write(vulkanBuffer, createInfo.data, createInfo.dataSize);
	}
	return VK_SUCCESS;
}

VkResult ng::vulkan::VulkanBufferRegionAllocator::write(VulkanBuffer * vulkanBuffer, void * newData, VkDeviceSize newDataSize)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_VulkanDevice->copyDataToBuffer(m_VkBuffer, vulkanBuffer->m_Offset, newDataSize, newData);
	auto it = m_Buffers.find(vulkanBuffer);
	it->second.buffer.m_Data = newData;
	it->second.buffer.m_DataSize = newDataSize;
	for (auto& copiedBuffer : it->second.copiedBuffers) {
		copiedBuffer->update(vulkanBuffer->m_Offset, vulkanBuffer->m_Size, newData, newDataSize);
	}
}

void ng::vulkan::VulkanBufferRegionAllocator::freeBuffer(VulkanBuffer* buffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	//print a VkDeviceSize printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters

	auto currentAllocIt = m_Buffers.find(buffer);
	if (currentAllocIt->second.copiedBuffers.size() > 1) {
		auto it = std::find(currentAllocIt->second.copiedBuffers.begin(), currentAllocIt->second.copiedBuffers.end(), buffer);
		currentAllocIt->second.copiedBuffers.erase(it);
		return;
	}
	else if (currentAllocIt->second.copiedBuffers.size() == 1) {
		auto it = std::find(currentAllocIt->second.copiedBuffers.begin(), currentAllocIt->second.copiedBuffers.end(), buffer);
		currentAllocIt->second.copiedBuffers.erase(it);
		if (currentAllocIt->second.buffer.m_Data != nullptr) {
			free(currentAllocIt->second.buffer.m_Data);
		}
	}
	else if (currentAllocIt->second.copiedBuffers.size() == 0) { //gets called when buffer goes out of scope just return- prevents never ending freeBuffer recursive loop
		if (currentAllocIt->second.buffer.m_Data != nullptr) {
			free(currentAllocIt->second.buffer.m_Data);
		}
		return;
	}
	

	if (currentAllocIt == m_Buffers.end()) {
		std::runtime_error("failed to find allocation to free\n");
	}

	//if this is neither the first nor the last allocation
	if ((currentAllocIt != m_Buffers.begin()) && (++currentAllocIt != m_Buffers.end())) {
		auto prevAllocIt = --currentAllocIt;
		auto nextAllocIt = ++currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second.buffer.m_Size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second.buffer.m_Size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.m_Size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second.buffer.m_Size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.m_Size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.buffer.m_Size != nextAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceOffset = currentAllocIt->first;
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.buffer.m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second.buffer.m_Size);
		}
	}
	//if this is the only allocation there is
	else if ((currentAllocIt == m_Buffers.begin()) && (++currentAllocIt == m_Buffers.end())) {
		m_FreeSpaces.clear();
		m_FreeSpaces.insert(0, m_MemorySize);
	}
	//if this is the first allocation
	else if (currentAllocIt == m_Buffers.begin()) {
		auto nextAllocIt = ++currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((currentAllocIt->first != 0) && (currentAllocIt->first + currentAllocIt->second.buffer.m_Size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(0, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (currentAllocIt->first != 0) {
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.m_Size;
			m_FreeSpaces.erase(0, prevFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.buffer.m_Size != nextAllocIt->first){
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.buffer.m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(0, currentAllocIt->second.buffer.m_Size);
		}
	}
	//if this is the last allocation
	else {
		auto prevAllocIt = --currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second.buffer.m_Size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second.buffer.m_Size != m_MemorySize)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.m_Size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second.buffer.m_Size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.m_Size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.buffer.m_Size != m_MemorySize) {
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.m_Size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.buffer.m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second.buffer.m_Size);
		}
	}

	//will call freeBuffer() again when ~VulkanBuffer() gets called, returns immediatly since bufferCount shall be 0 then
	m_Buffers.erase(currentAllocIt);
}

void ng::vulkan::VulkanBufferRegionAllocator::defragment(VkCommandBuffer defragCommandBuffer, std::unique_lock<std::mutex>* lock)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	m_FreeSpaces.clear();

	VkDeviceSize offset = 0;
	void* data;

	//std::vector<std::pair<VulkanBufferCreateInfo, std::vector<VulkanBuffer*>>> newBuffers(Buffers.size());
	std::vector<VkBufferCopy> copyRegions(m_Buffers.size());

	Buffers newBuffers;

	VkDeviceSize lastOffset;

	int i = 0;
	for (auto it = m_Buffers.begin(); it != m_Buffers.end(); ++it ) {
		
		copyRegions.emplace_back(it->second.buffer.m_Offset, offset, it->second.buffer.m_DataSize);

		VulkanBuffer::VulkanBufferInternalCreateInfo createInternalInfo;

		createInternalInfo.bufferRegionAllocator = this;
		createInternalInfo.offset = offset;
		createInternalInfo.size = it->second.buffer.m_Size;
		createInternalInfo.vkBuffer = this->m_VkBuffer;
		createInternalInfo.data = it->second.buffer.m_Data;
		createInternalInfo.dataSize = it->second.buffer.m_DataSize;
		createInternalInfo.onUpdateCallback = it->second.buffer.m_OnUpdateCallback;

		auto it2 = newBuffers.emplace(createInternalInfo);
		newBuffers.setBufferCopies(&it->second.buffer ,&it->second.copiedBuffers);

		lastOffset = offset + it->second.buffer.m_Size;
		offset += it->second.buffer.m_Size;
		++i;
	}

	if (lock != nullptr) {
		lock->lock();
	}

	m_Buffers.clear();
	m_FreeSpaces.clear();
	m_Buffers = newBuffers;

	for (auto& originalBuffer : m_Buffers) {
		for (auto& buffer : originalBuffer.second.copiedBuffers) {
			buffer->update(
				originalBuffer.second.buffer.m_Offset,
				originalBuffer.second.buffer.m_Size,
				originalBuffer.second.buffer.m_Data,
				originalBuffer.second.buffer.m_DataSize
			);
		}
	}

	m_FreeSpaces.insert(lastOffset, m_MemorySize - lastOffset);

	vkCmdCopyBuffer(defragCommandBuffer, m_VkBuffer, m_VkBuffer, copyRegions.size(), copyRegions.data());
}

