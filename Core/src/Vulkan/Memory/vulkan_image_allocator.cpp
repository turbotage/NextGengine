#include "vulkan_image_allocator.h"


std::forward_list<ng::vulkan::VulkanImageChunk>::iterator ng::vulkan::VulkanImageAllocator::addChunk(std::forward_list<VulkanImageChunk>* chunks, VkResult * result)
{
	auto it = &chunks->emplace_front(m_StandardChunkSize, m_MemoryAlignment);
	VkResult res = 
}

VkDeviceSize ng::vulkan::VulkanImageAllocator::getAlignedSize(VkDeviceSize size) {
	//should never be read and written to at the same time so no data race, no need to lock mutex
	if (size % m_MemoryAlignment != 0) {
		size = size + (m_MemoryAlignment - (size % m_MemoryAlignment));
	}
}

ng::vulkan::VulkanImageAllocator::VulkanImageAllocator(VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkMemoryAlignment alignment, VkDeviceSize standardAllocSize) {
	m_VulkanDevice = vulkanDevice;
	m_MemoryFlags = flags;
	m_MemoryAlignment = alignment;
	m_StandardChunkSize = standardAllocSize;
}

void ng::vulkan::VulkanImageAllocator::createImage(VulkanImageCreateInfo createInfo, VulkanImage * image)
{
	if (m_MemoryFlags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {

	}
	else if (m_MemoryFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

	}

	bool hasStaging = false;
	bool inDeviceMemory = false;

	//staging-mem
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
				VulkanAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.size;
				image->m_StagingAllocation = chunk.allocate(*freeBlockIt, allocInfo);
				image->m_StagingChunk = &chunk;

				//TODO write createInfo.data to stagingBuffer
				hasStaging = true;
				break;
			}

		}
		if (!hasStaging) {
			//if this is the second time we arives here the program can't continue from here
			if (i == 1) {
				LOGD("couldn't find staging memory for vulkanBuffer");
				debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
			}
			VkResult result;
			auto it = addChunk(&m_StagingChunks, &result);
			if (result != VK_SUCCESS) {

				defragmentStagingMem();
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.size;
				image->m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
				image->m_StagingChunk = &*it;

				//TODO write createInfo.data to stagingBuffer
				hasStaging = true;
			}

			//should try to defrag chunks and then try again
			LOGD("failed to create Staging Buffer")
		}
	}

	//device-memory
	for (int i = 0; i < 2 && !hasStaging; ++i) {
		for (auto& chunk : m_DeviceChunks) {

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.size;
				image->m_DeviceAllocation = chunk.allocate(*freeBlockIt, allocInfo);
				image->m_DeviceChunk = &chunk;

				image->moveToDevice();

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
				VulkanAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.size;
				image->m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
				image->m_StagingChunk = &*it;

				image->moveToDevice();

				//TODO write createInfo.data to stagingBuffer
				hasStaging = true;
			}

			//should try to defrag chunks and then try again
			LOGD("failed to create Staging Buffer")
		}
	}

}

void ng::vulkan::VulkanImageAllocator::defragmentDeviceMem(uint16 chunksDefragNum, bool waitUntilComplete)
{
	
}

void ng::vulkan::VulkanImageAllocator::defragmentStagingMem(uint16 chunksDefragNum, bool waitUntilComplete)
{

}

