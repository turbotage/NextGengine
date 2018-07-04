#include "vulkan_buffer_allocator.h"

VkDeviceSize ng::vulkan::VulkanBufferAllocator::getAlignedSize(VkDeviceSize size) {
	//should never be read and written to at the same time so no data race, no need to lock mutex
	if (size % m_MemoryAlignment != 0) {
		size = size + (m_MemoryAlignment - (size % m_MemoryAlignment));
	}
}

VkResult ng::vulkan::VulkanBufferAllocator::createDeviceLocal(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer) {
	bool hasStaging = false;
	bool inDeviceMemory = false;

	createInfo.size = getAlignedSize(createInfo.size);

	//staging-memory 
	for (int i = 0; i < 2 && !hasStaging; ++i) {
		for (auto& chunk : m_StagingChunks) {

			if (createInfo.size >= chunk.getTotalFreeSpace()) {
				if (createInfo.size != chunk.getTotalFreeSpace()) {
					continue;
				}
				else if (chunk.getNumFreeSpaces() != 1) {
					continue;
				}
			}

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
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
			//if this is the second time we arives here the program can't continue from here
			if (i == 2) {
				LOGD("couldn't find staging memory for vulkanBuffer");
				debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
			}
			VkResult result;
			auto it = addChunk(&m_StagingChunks, &result);
			if (result != VK_SUCCESS) {

				for (auto& chunk : m_StagingChunks) {
					if (chunk.getTotalFreeSpace() >= createInfo.size) {
						chunk.defragment(m_VulkanDevice);
					}
				}
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
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
	for (int i = 0; i < 2 && !hasStaging; ++i) {
		for (auto& chunk : m_DeviceChunks) {

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.dataSize;
				buffer->m_DeviceAllocation = chunk.allocate(*freeBlockIt, allocInfo);
				buffer->m_DeviceChunk = &chunk;

				//TODO write data in stagingBuffer to deviceMemory
				inDeviceMemory = true;
				//done
				return;
			}

		}
		if (!inDeviceMemory) {
			//if this is the second time we arives here the program can't continue from here
			if (i == 2) {
				LOGD("couldn't find staging memory for vulkanBuffer");
				debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
			}
			VkResult result;
			auto it = addChunk(&m_DeviceChunks, &result);
			if (result != VK_SUCCESS) {

				for (auto& chunk : m_StagingChunks) {
					if (chunk.getTotalFreeSpace() >= createInfo.size) {
						chunk.defragment(m_VulkanDevice);
					}
				}
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
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

}

VkResult ng::vulkan::VulkanBufferAllocator::createMappableDeviceLocal(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer) {
	bool hasStaging = false;
	bool inDeviceMemory = false;

	createInfo.size = getAlignedSize(createInfo.size);

	//staging-memory 
	for (int i = 0; i < 2 && !hasStaging; ++i) {
		for (auto& chunk : m_StagingChunks) {

			if (createInfo.size >= chunk.getTotalFreeSpace()) {
				if (createInfo.size != chunk.getTotalFreeSpace()) {
					continue;
				}
				else if (chunk.getNumFreeSpaces() != 1) {
					continue;
				}
			}

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
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
			//if this is the second time we arives here the program can't continue from here
			if (i == 2) {
				LOGD("couldn't find staging memory for vulkanBuffer");
				debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
			}
			VkResult result;
			auto it = addChunk(&m_StagingChunks, &result);
			if (result != VK_SUCCESS) {

				for (auto& chunk : m_StagingChunks) {
					if (chunk.getTotalFreeSpace() >= createInfo.size) {
						chunk.defragment(m_VulkanDevice);
					}
				}
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
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
	for (int i = 0; i < 2 && !hasStaging; ++i) {
		for (auto& chunk : m_DeviceChunks) {

			auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

			if (freeBlockIt != chunk.freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
				allocInfo.size = createInfo.size;
				allocInfo.dataSize = createInfo.dataSize;
				buffer->m_DeviceAllocation = chunk.allocate(*freeBlockIt, allocInfo);
				buffer->m_DeviceChunk = &chunk;

				//TODO write data in stagingBuffer to deviceMemory
				inDeviceMemory = true;
				//done
				return;
			}

		}
		if (!inDeviceMemory) {
			//if this is the second time we arives here the program can't continue from here
			if (i == 2) {
				LOGD("couldn't find staging memory for vulkanBuffer");
				debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
			}
			VkResult result;
			auto it = addChunk(&m_DeviceChunks, &result);
			if (result != VK_SUCCESS) {

				for (auto& chunk : m_StagingChunks) {
					if (chunk.getTotalFreeSpace() >= createInfo.size) {
						chunk.defragment(m_VulkanDevice);
					}
				}
				continue;
			}

			auto freeBlockIt = it->getClosestMatch(createInfo.size);

			if (freeBlockIt != it->freeBlocks.end()) {
				VulkanAllocationCreateInfo allocInfo;
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

}

ng::vulkan::VulkanBufferAllocator::VulkanBufferAllocator(VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkMemoryAlignment alignment, VkDeviceSize standardAllocSize) {
	m_VulkanDevice = vulkanDevice;
	m_MemoryFlags = flags;
	m_MemoryAlignment = alignment;
	m_StandardChunkSize = standardAllocSize;
}

void ng::vulkan::VulkanBufferAllocator::createBuffer(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer) {

	if (m_MemoryFlags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
		createDeviceLocal(createInfo, buffer);
	}
	else if (m_MemoryFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
		createMappableDeviceLocal(createInfo, buffer);
	}
}