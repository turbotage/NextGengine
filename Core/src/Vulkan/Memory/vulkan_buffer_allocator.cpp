#include "vulkan_buffer_allocator.h"
#include "vulkan_buffer.h"

std::list<ng::vulkan::VulkanBufferChunk>::iterator ng::vulkan::VulkanBufferAllocator::addChunk(std::list<VulkanBufferChunk>* chunks, VkResult* result) {
	auto it = &chunks->emplace_front(m_StandardChunkSize, m_MemoryAlignment);
	VkResult res = it->create(m_VulkanDevice, m_MemoryFlags, m_BufferUsage);
	if (result != nullptr) {
		*result = res;
	}
}
/*
VkDeviceSize ng::vulkan::VulkanBufferAllocator::getAlignedSize(VkDeviceSize size) {
	//should never be read and written to at the same time so no data race, no need to lock mutex
	if (size % m_MemoryAlignment != 0) {
		size = size + (m_MemoryAlignment - (size % m_MemoryAlignment));
	}
}
*/

ng::vulkan::VulkanBufferAllocator::VulkanBufferAllocator()
{
}

ng::vulkan::VulkanBufferAllocator::VulkanBufferAllocator(VulkanBufferAllocatorCreateInfo createInfo)
{
	create(createInfo);
}

void ng::vulkan::VulkanBufferAllocator::create(VulkanBufferAllocatorCreateInfo createInfo)
{
	m_VulkanDevice = createInfo.vulkanDevice;
	m_MemoryFlags = createInfo.memoryFlags;
	m_MemoryAlignment = createInfo.alignment;
	m_StandardChunkSize = createInfo.standardChunkSize;
}

void ng::vulkan::VulkanBufferAllocator::createBuffer(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer) {

	if (m_MemoryFlags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
		/* DeviceMemory : { Heap 0, MemoryType 0 }, StagingMemory : { Heap 2, MemoryType 2 } */
		//createDeviceLocal(createInfo, buffer);
	}
	else if (m_MemoryFlags == (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
		/* DeviceMemory : { Heap 1, MemoryType 1 }, StagingMemory : { Heap 2, MemoryType 2 }, */
		//createMappableDeviceLocal(createInfo, buffer);
	}

	bool hasStaging = false;
	bool inDeviceMemory = false;

	//staging-memory 
	for (int i = 0; i < 2 && !hasStaging; ++i) {
		for (auto& chunk : m_StagingChunks) {

			if (createInfo.size >= chunk.getTotalFreeSpace()) {
				if (createInfo.size != chunk.getTotalFreeSpace()) {
					continue;
				}
				else if (chunk.getNumFreeSpaces() != 1) { // if the available size is exactly the required but it isn't from one block then it wont fit
					continue;
				}
			}

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanBufferAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.dataSize;
				buffer->m_StagingAllocation = chunk.allocate(*freeBlockIt, allocInfo);
				buffer->m_StagingChunk = &chunk;

				//TODO write createInfo.data to stagingBuffer
				hasStaging = true;
				break;
			}

		}

		if (!hasStaging) {
			/* if this is the second time we arives here the program can't continue from here,
			we couldn't add another chunk and no new allocation could be found after a full defragmentation */
			if (i == 1) {
				LOGD("couldn't find staging memory for vulkanBuffer");
				tools::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
			}
			VkResult result;
			auto it = addChunk(&m_StagingChunks, &result);
			if (result != VK_SUCCESS) {

				defragmentStagingMem();
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanBufferAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.dataSize;
				buffer->m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
				buffer->m_StagingChunk = &*it;

				//TODO write createInfo.data to stagingBuffer
				hasStaging = true;
			}

			//should try to defrag chunks and then try again
			LOGD("failed to create Staging Buffer")
		}
	}

	//device-memory
	for (int i = 0; i < 2 && !inDeviceMemory; ++i) {
		for (auto& chunk : m_DeviceChunks) {

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanBufferAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.dataSize;
				buffer->m_DeviceAllocation = chunk.allocate(*freeBlockIt, allocInfo);
				buffer->m_DeviceChunk = &chunk;

				buffer->moveToDevice();

				//TODO write data in stagingBuffer to deviceMemory
				inDeviceMemory = true;
				//done
				return;
			}

		}
		if (!inDeviceMemory) {
			//if this is the second time we arives here the program can't continue from here
			if (i == 1) {
				LOGD("couldn't find device-memory for vulkanBuffer");
				//debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
				break; // no need to continue we can't do more
			}
			VkResult result;
			auto it = addChunk(&m_DeviceChunks, &result);
			if (result != VK_SUCCESS) {

				defragmentDeviceMem();
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanBufferAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.dataSize;
				buffer->m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
				buffer->m_StagingChunk = &*it;

				buffer->moveToDevice();

				//TODO write createInfo.data to stagingBuffer
				hasStaging = true;
			}

			//should try to defrag chunks and then try again
			LOGD("failed to create Staging Buffer")
		}
	}

}

void ng::vulkan::VulkanBufferAllocator::defragmentDeviceMem(uint16 chunksDefragNum, bool waitUntilComplete)
{
	if (chunksDefragNum > m_DeviceChunks.size()) {
		chunksDefragNum = m_DeviceChunks.size();
	}

	std::vector<std::vector<VkBufferCopy>> listOfCopyRegions(chunksDefragNum);
	
	std::vector<VkCommandBuffer> commandBuffers = m_VulkanDevice->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, chunksDefragNum,
		m_VulkanDevice->memoryCommandPool, true);

	
	int i = 0;
	for (auto it = m_DeviceChunks.begin(); (i < chunksDefragNum) && (it != m_DeviceChunks.end()); ++it) {
		it->defragment(&listOfCopyRegions[i]);

		vkCmdCopyBuffer(commandBuffers[i], it->buffer, it->buffer, listOfCopyRegions[i].size(), reinterpret_cast<VkBufferCopy*>(listOfCopyRegions[i].data()));
		
		VULKAN_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[i]));

		VkSubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[i];

		VULKAN_CHECK_RESULT(vkQueueSubmit(m_VulkanDevice->transferQueue, 1, &submitInfo, VK_NULL_HANDLE));

		++i;
	}

	if (waitUntilComplete) {
		vkQueueWaitIdle(m_VulkanDevice->transferQueue);
	}

}

void ng::vulkan::VulkanBufferAllocator::defragmentStagingMem(uint16 chunksDefragNum, bool waitUntilComplete)
{
	if (chunksDefragNum > m_StagingChunks.size()) {
		chunksDefragNum = m_StagingChunks.size();
	}

	std::vector<std::vector<VkBufferCopy>> listOfCopyRegions(chunksDefragNum);

	std::vector<VkCommandBuffer> commandBuffers = m_VulkanDevice->createCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, chunksDefragNum,
		m_VulkanDevice->memoryCommandPool, true);

	int i = 0;
	for (auto it = m_StagingChunks.begin(); (i < chunksDefragNum) && (it != m_StagingChunks.end()); ++it) {
		it->defragment(&listOfCopyRegions[i]);

		vkCmdCopyBuffer(commandBuffers[i], it->buffer, it->buffer, listOfCopyRegions[i].size(), reinterpret_cast<VkBufferCopy*>(listOfCopyRegions[i].data()));

		VULKAN_CHECK_RESULT(vkEndCommandBuffer(commandBuffers[i]));

		VkSubmitInfo submitInfo = {};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[i];

		VULKAN_CHECK_RESULT(vkQueueSubmit(m_VulkanDevice->transferQueue, 1, &submitInfo, VK_NULL_HANDLE));

		++i;
	}

	if (waitUntilComplete) {
		vkQueueWaitIdle(m_VulkanDevice->transferQueue);
	}

}
