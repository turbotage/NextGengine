#include "../vulkan_device.h"
#include "vulkan_buffer_region_allocator.h"

//VkBufferRegionAllocator

ng::vulkan::VulkanBufferRegionAllocator::VulkanBufferRegionAllocator(VulkanBufferRegionAllocatorCreateInfo createInfo)
	: m_VulkanDevice(createInfo.vulkanDevice), m_MemorySize(createInfo.memorySize), m_MemoryAlignment(createInfo.memoryAlignment)
{
	m_FreeMemorySize = m_MemorySize;
}

uint32 ng::vulkan::VulkanBufferRegionAllocator::increaseBufferCopies(VulkanBuffer * buffer)
{
	return m_Buffers.increaseBufferCopies(buffer);
}

std::pair<VkDeviceSize, VkDeviceSize> ng::vulkan::VulkanBufferRegionAllocator::findSuitableFreeSpace(VkDeviceSize size)
{
	return m_FreeSpaces.findSuitableFreeSpace(size);
}

uint32 ng::vulkan::VulkanBufferRegionAllocator::getFreeSpaceCount()
{
	return m_FreeSpaces.size();
}

bool ng::vulkan::VulkanBufferRegionAllocator::isInBufferRegion(VulkanBuffer* buffer)
{

	if (*buffer->VkBuffer == this->vkBuffer) {
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

ng::vulkan::VulkanBuffer ng::vulkan::VulkanBufferRegionAllocator::createBuffer(VkDeviceSize size)
{

	VkDeviceSize allocSize = size + m_MemoryAlignment -(size % m_MemoryAlignment); //fix size to right mem-alignment

	auto freeSpaceRet = m_FreeSpaces.findSuitableFreeSpace(allocSize);

	VulkanBufferCreateInfo createInfo;
	createInfo.bufferRegionAllocator = this;
	createInfo.offset = freeSpaceRet.first;
	createInfo.size = allocSize;
	createInfo.vkBuffer = &vkBuffer;

	//printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", buffer.offset, buffer.size); //show allocation parameters
	auto it = m_Buffers.emplace(createInfo);

	//buffer.setOffsetAndSizeIterator(allocationRet);

	m_FreeSpaces.erase(freeSpaceRet.first, freeSpaceRet.second); //remove the old freespace
																 //insert the new freespace
	m_FreeSpaces.insert(createInfo.offset + createInfo.size, freeSpaceRet.second - createInfo.size);
	m_FreeMemorySize -= allocSize;
	return it.first->second.buffer;
}

ng::vulkan::VulkanBuffer ng::vulkan::VulkanBufferRegionAllocator::createBuffer(VkDeviceSize freeSpaceOffset, VkDeviceSize freeSpaceSize, VkDeviceSize size)
{
	VulkanBufferCreateInfo createInfo;
	createInfo.bufferRegionAllocator = this;
	createInfo.offset = freeSpaceOffset;
	createInfo.size = size;
	createInfo.vkBuffer = &vkBuffer;

	auto it = m_Buffers.emplace(createInfo);

	m_FreeSpaces.erase(freeSpaceOffset, freeSpaceSize);
	m_FreeSpaces.insert(freeSpaceOffset + size, freeSpaceSize - size);
	return it.first->second.buffer;
}

void ng::vulkan::VulkanBufferRegionAllocator::freeBuffer(VulkanBuffer* buffer)
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
		if (currentAllocIt->second.buffer.Data != nullptr) {
			free(currentAllocIt->second.buffer.Data);
		}
	}
	else if (currentAllocIt->second.copiedBuffers.size() == 0) { //gets called when buffer goes out of scope just return- prevents never ending freeBuffer recursive loop
		if (currentAllocIt->second.buffer.Data != nullptr) {
			free(currentAllocIt->second.buffer.Data);
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
		if ((prevAllocIt->first + prevAllocIt->second.buffer.Size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second.buffer.Size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.Size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.Size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second.buffer.Size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.Size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.buffer.Size != nextAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceOffset = currentAllocIt->first;
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.buffer.Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second.buffer.Size);
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
		if ((currentAllocIt->first != 0) && (currentAllocIt->first + currentAllocIt->second.buffer.Size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.Size + nextFreeSpace.second;
			m_FreeSpaces.erase(0, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (currentAllocIt->first != 0) {
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.Size;
			m_FreeSpaces.erase(0, prevFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.buffer.Size != nextAllocIt->first){
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.buffer.Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(0, currentAllocIt->second.buffer.Size);
		}
	}
	//if this is the last allocation
	else {
		auto prevAllocIt = --currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second.buffer.Size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second.buffer.Size != m_MemorySize)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.Size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.Size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second.buffer.Size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.buffer.Size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.buffer.Size != m_MemorySize) {
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.buffer.Size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.buffer.Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second.buffer.Size);
		}
	}

	//will call freeBuffer() again when ~VulkanBuffer() gets called, returns immediatly since bufferCount shall be 0 then
	m_Buffers.erase(currentAllocIt);
}

void ng::vulkan::VulkanBufferRegionAllocator::defragment(VkCommandBuffer defragCommandBuffer)
{
	m_FreeSpaces.clear();

	VkDeviceSize offset = 0;
	void* data;

	//std::vector<std::pair<VulkanBufferCreateInfo, std::vector<VulkanBuffer*>>> newBuffers(Buffers.size());
	std::vector<VkBufferCopy> copyRegions(m_Buffers.size());

	Buffers newBuffers;

	VkDeviceSize lastOffset;

	int i = 0;
	for (auto it = m_Buffers.begin(); it != m_Buffers.end(); ++it ) {
		
		copyRegions.emplace_back(it->second.buffer.Offset, offset, it->second.buffer.DataSize);

		VulkanBufferCreateInfo createInfo = {
			this,
			offset,
			it->second.buffer.Size,
			it->second.buffer.DataSize,
			&this->vkBuffer,
			it->second.buffer.Data,
			it->second.buffer.OnUpdate
		};

		auto it2 = newBuffers.emplace(createInfo);
		newBuffers.setBufferCopies(&it->second.buffer ,&it->second.copiedBuffers);

		lastOffset = offset + it->second.buffer.Size;
		offset += it->second.buffer.Size;
		++i;
	}

	m_Buffers.clear();
	m_FreeSpaces.clear();
	m_Buffers = newBuffers;

	for (auto& buffer : m_Buffers) {
		buffer.second.buffer.OnUpdate();
	}

	m_FreeSpaces.insert(lastOffset, m_MemorySize - lastOffset);

	vkCmdCopyBuffer(defragCommandBuffer, vkBuffer, vkBuffer, copyRegions.size(), copyRegions.data());
}

