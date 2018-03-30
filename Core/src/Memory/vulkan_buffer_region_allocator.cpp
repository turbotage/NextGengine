#include "../Graphics/vulkan_device.h"
#include "vulkan_buffer_region_allocator.h"

//VkBufferRegionAllocator

ng::memory::vma::VulkanBufferRegionAllocator::VulkanBufferRegionAllocator(VulkanBufferRegionAllocatorCreateInfo createInfo)
{
	m_VulkanDevice = createInfo.vulkanDevice;
	m_StagingBuffer = createInfo.stagingBuffer;
	m_StagingBufferMemory = createInfo.stagingBufferMemory;
	m_MemorySize = createInfo.memorySize;
	m_FreeMemorySize = m_MemorySize;
	m_MemoryAlignment = createInfo.memoryAlignment;
}

uint32 ng::memory::vma::VulkanBufferRegionAllocator::increaseBufferCopies(VulkanBuffer * buffer)
{
	return m_Buffers.increaseBufferCopies(buffer);
}

std::pair<VkDeviceSize, VkDeviceSize> ng::memory::vma::VulkanBufferRegionAllocator::findSuitableFreeSpace(VkDeviceSize size)
{
	return m_FreeSpaces.findSuitableFreeSpace(size);
}

uint32 ng::memory::vma::VulkanBufferRegionAllocator::getFreeSpaceCount()
{
	return m_FreeSpaces.size();
}

bool ng::memory::vma::VulkanBufferRegionAllocator::isInBufferRegion(VulkanBuffer* buffer)
{
	if (*buffer->m_VkBuffer == this->buffer) {
		return true;
	}
	/*
	auto bufferIt = m_Buffers.find(buffer);
	if (bufferIt != m_Buffers.end()) {
		if (bufferIt->second.buffer == *buffer) {
			return true;
		}
	}
	*/
	return false;
}

ng::memory::VulkanBuffer ng::memory::vma::VulkanBufferRegionAllocator::createBuffer(VkDeviceSize size)
{

	VkDeviceSize allocSize = size + m_MemoryAlignment -(size % m_MemoryAlignment); //fix size to right mem-alignment

	auto freeSpaceRet = m_FreeSpaces.findSuitableFreeSpace(allocSize);

	VulkanBufferCreateInfo createInfo;
	createInfo.bufferRegionAllocator = this;
	createInfo.offset = freeSpaceRet.first;
	createInfo.size = allocSize;
	createInfo.vkBuffer = &buffer;

	//printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", buffer.offset, buffer.size); //show allocation parameters
	auto it = m_Buffers.emplace(createInfo);

	//buffer.setOffsetAndSizeIterator(allocationRet);

	m_FreeSpaces.erase(freeSpaceRet.first, freeSpaceRet.second); //remove the old freespace
																 //insert the new freespace
	m_FreeSpaces.insert(createInfo.offset + createInfo.size, freeSpaceRet.second - createInfo.size);
	m_FreeMemorySize -= allocSize;
	return it.first->second.buffer;
}

ng::memory::VulkanBuffer ng::memory::vma::VulkanBufferRegionAllocator::createBuffer(VkDeviceSize freeSpaceOffset, VkDeviceSize freeSpaceSize, VkDeviceSize size)
{
	VulkanBufferCreateInfo createInfo;
	createInfo.bufferRegionAllocator = this;
	createInfo.offset = freeSpaceOffset;
	createInfo.size = size;
	createInfo.vkBuffer = &buffer;

	auto it = m_Buffers.emplace(createInfo);

	m_FreeSpaces.erase(freeSpaceOffset, freeSpaceSize);
	m_FreeSpaces.insert(freeSpaceOffset + size, freeSpaceSize - size);
	return it.first->second.buffer;
}

void ng::memory::vma::VulkanBufferRegionAllocator::freeBuffer(VulkanBuffer* buffer)
{

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

void ng::memory::vma::VulkanBufferRegionAllocator::defragment(VkCommandBuffer defragCommandBuffer)
{
	m_FreeSpaces.clear();

	VkDeviceSize offset = 0;
	void* data;

	std::vector<std::pair<VulkanBufferCreateInfo, std::vector<VulkanBuffer*>>> newBuffers(m_Buffers.size());

	std::pair<VulkanBufferCreateInfo, std::vector<VulkanBuffer*>> tmpCreationPair;

	byte* dataPtr;

	vkMapMemory(m_VulkanDevice->logicalDevice, m_StagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
	dataPtr = (byte*)data;

	for (auto it = m_Buffers.begin(); it != m_Buffers.end(); ++it ) {
		if (it->first != offset) {
			memcpy((void*)dataPtr, it->second.buffer.m_Data, it->second.buffer.m_Size);
			tmpCreationPair.first.offset = offset;
			tmpCreationPair.first.size = it->second.buffer.m_Size;
			tmpCreationPair.first.vkBuffer = &this->buffer;
			tmpCreationPair.first.bufferRegionAllocator = this;
			tmpCreationPair.second = it->second.copiedBuffers;

			newBuffers.push_back(tmpCreationPair);
		}
	}
	vkUnmapMemory(m_VulkanDevice->logicalDevice, m_StagingBufferMemory);

	m_Buffers.clear();

	for (int i = 0; i < newBuffers.size(); ++i) {
		auto it = m_Buffers.emplace(newBuffers[i].first);
		m_Buffers.setBufferCopies(&it.first->second.buffer, &newBuffers[i].second);
		for (auto buffer : newBuffers[i].second) {
			buffer->m_Offset = it.first->second.buffer.m_Offset;
			buffer->update();
		}
	}

	VkDeviceSize newFreeSpaceOffset = (--m_Buffers.end())->first + (--m_Buffers.end())->second.buffer.m_Size;
	VkDeviceSize newFreeSpaceSize = (--m_Buffers.end())->second.buffer.m_Size;

	m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = m_MemorySize - newFreeSpaceSize;
	vkCmdCopyBuffer(defragCommandBuffer, m_StagingBuffer, buffer, 1, &copyRegion);

	vkEndCommandBuffer(defragCommandBuffer);
}

