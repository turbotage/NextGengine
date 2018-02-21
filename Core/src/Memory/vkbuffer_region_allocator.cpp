#include "vkbuffer_region_allocator.h"

//VkBufferRegionAllocator

ng::memory::vma::VkBufferRegionAllocator::VkBufferRegionAllocator(VkBufferRegionAllocatorCreateInfo createInfo)
	: m_CreateInfo(createInfo)
{

}

std::pair<VkDeviceSize, VkDeviceSize> ng::memory::vma::VkBufferRegionAllocator::findSuitableFreeSpace(VkDeviceSize size)
{
	return m_FreeSpaces.findSuitableFreeSpace(size);
}

bool ng::memory::vma::VkBufferRegionAllocator::isInBufferRegion(Buffer* buffer)
{
	auto bufferIt = m_Buffers.find(buffer);
	if (bufferIt != m_Buffers.end()) {
		if (bufferIt->second == buffer) {
			return true;
		}
	}
	return false;
}

ng::memory::Buffer* ng::memory::vma::VkBufferRegionAllocator::createBuffer(VkDeviceSize size)
{

	VkDeviceSize allocSize = size + m_CreateInfo.memoryAlignment -(size % m_CreateInfo.memoryAlignment); //fix size to right mem-alignment

	auto freeSpaceRet = m_FreeSpaces.findSuitableFreeSpace(allocSize);
	Buffer* retBuffer = new Buffer(freeSpaceRet.first, freeSpaceRet.second, &buffer);

	//printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", buffer.offset, buffer.size); //show allocation parameters
	m_Buffers.insert(retBuffer);

	//buffer.setOffsetAndSizeIterator(allocationRet);

	m_FreeSpaces.erase(freeSpaceRet.first, freeSpaceRet.second); //remove the old freespace
																 //insert the new freespace
	m_FreeSpaces.insert(retBuffer->m_Offset + retBuffer->m_Size, freeSpaceRet.second - retBuffer->m_Size);
	m_FreeMemorySize -= allocSize;
	return retBuffer;
}

ng::memory::Buffer* ng::memory::vma::VkBufferRegionAllocator::createBuffer(VkDeviceSize offset, VkDeviceSize size)
{
	Buffer* retBuffer = new Buffer(offset, size, &buffer);

	m_Buffers.insert(retBuffer);

	m_FreeSpaces.erase(offset, size);
	m_FreeSpaces.insert(retBuffer->m_Offset + retBuffer->m_Size, size - retBuffer->m_Size);
	return retBuffer;
}

void ng::memory::vma::VkBufferRegionAllocator::freeBuffer(Buffer* buffer)
{

	//print a VkDeviceSize printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters

	if (buffer->m_BufferCopies != 0) {
		buffer->m_BufferCopies--;
		return;
	}

	auto currentAllocIt = m_Buffers.find(buffer->m_Offset);
	if (currentAllocIt == m_Buffers.end()) {
		std::runtime_error("failed to find allocation to free\n");
	}

	//if this is neither the first nor the last allocation
	if ((currentAllocIt != m_Buffers.begin()) && (++currentAllocIt != m_Buffers.end())) {
		auto prevAllocIt = --currentAllocIt;
		auto nextAllocIt = ++currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second->m_Size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second->m_Size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second->m_Size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second->m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second->m_Size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second->m_Size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second->m_Size != nextAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceOffset = currentAllocIt->first;
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second->m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second->m_Size);
		}
	}
	//if this is the only allocation there is
	else if ((currentAllocIt == m_Buffers.begin()) && (++currentAllocIt == m_Buffers.end())) {
		m_FreeSpaces.clear();
		m_FreeSpaces.insert(0, m_CreateInfo.memorySize);
	}
	//if this is the first allocation
	else if (currentAllocIt == m_Buffers.begin()) {
		auto nextAllocIt = ++currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((currentAllocIt->first != 0) && (currentAllocIt->first + currentAllocIt->second->m_Size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second->m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(0, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (currentAllocIt->first != 0) {
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second->m_Size;
			m_FreeSpaces.erase(0, prevFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second->m_Size != nextAllocIt->first){
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second->m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(0, currentAllocIt->second->m_Size);
		}
	}
	//if this is the last allocation
	else {
		auto prevAllocIt = --currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second->m_Size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second->m_Size != m_CreateInfo.memorySize)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second->m_Size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second->m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second->m_Size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second->m_Size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second->m_Size != m_CreateInfo.memorySize) {
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second->m_Size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second->m_Size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second->m_Size);
		}
	}
	
	free(currentAllocIt->second->m_Data);
	
	m_Buffers.erase(currentAllocIt);
}

void ng::memory::vma::VkBufferRegionAllocator::defragment()
{
	m_FreeSpaces.clear();

	VkDeviceSize offset = 0;
	void* data;

	std::vector<Buffer*> newBuffers(m_Buffers.size());

	Buffer* movedBuffer;

	byte* dataPtr;

	vkMapMemory(*m_CreateInfo.device, *m_CreateInfo.stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
	dataPtr = (byte*)data;

	for (auto it = m_Buffers.begin(); it != m_Buffers.end(); ++it ) {
		if (it->first != offset) {
			memcpy((void*)dataPtr, it->second->m_Data, it->second->m_Size);
			movedBuffer = it->second;
			movedBuffer->m_Offset = offset;
			newBuffers.push_back(movedBuffer);
		}
	}
	vkUnmapMemory(*m_CreateInfo.device, *m_CreateInfo.stagingBufferMemory);

	m_Buffers.clear();

	for (int i = 0; i < newBuffers.size(); ++i) {
		m_Buffers.insert(newBuffers[i]);
		newBuffers[i]->update();
	}

	VkDeviceSize newFreeSpaceOffset = (--m_Buffers.end())->first + (--m_Buffers.end())->second->m_Size;
	VkDeviceSize newFreeSpaceSize = (--m_Buffers.end())->second->m_Size;

	m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = *m_CreateInfo.commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(*m_CreateInfo.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = m_CreateInfo.memorySize - newFreeSpaceSize;
	vkCmdCopyBuffer(commandBuffer, *m_CreateInfo.stagingBuffer, buffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(*m_CreateInfo.queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(*m_CreateInfo.queue);

	vkFreeCommandBuffers(*m_CreateInfo.device, *m_CreateInfo.commandPool, 1, &commandBuffer);
}

