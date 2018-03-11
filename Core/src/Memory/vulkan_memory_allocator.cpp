
#include "../Graphics/vulkan_device.h"
#include "../debug.h"
#include "vulkan_memory_allocator.h"

#define DEFAULT_ALLOC_SIZE 536870912

VkResult ng::memory::vma::VulkanMemoryAllocator::createBFA()
{
	VulkanBufferRegionAllocatorCreateInfo bfaCreateInfo;
	bfaCreateInfo.vulkanDevice = m_VulkanDevice;
	bfaCreateInfo.stagingBuffer = m_StagingBuffer;
	bfaCreateInfo.stagingBufferMemory = m_StagingBufferMemory;
	bfaCreateInfo.memorySize = defaultAllocationSize;
	bfaCreateInfo.memoryAlignment = memoryAlignment;
	bfaCreateInfo.commandPool = m_CommandPool;
	bfaCreateInfo.queue = m_Queue;

	VulkanBufferRegionAllocator* bfa = new VulkanBufferRegionAllocator(bfaCreateInfo);
	VkResult res = m_VulkanDevice->createBuffer(usage, memoryProperties, defaultAllocationSize, &bfa->buffer, &bfa->bufferMemory);
	if (res != VK_SUCCESS) {
		delete bfa;
	}
	else {
		m_BufferRegionAllocators.push_back(bfa);
	}

	return res;
}

ng::memory::vma::VulkanMemoryAllocator::VulkanMemoryAllocator()
{

}

ng::memory::vma::VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocatorCreateInfo createInfo)
{
	init(createInfo);
}

ng::memory::vma::VulkanMemoryAllocator::~VulkanMemoryAllocator()
{

}

void ng::memory::vma::VulkanMemoryAllocator::init(VulkanMemoryAllocatorCreateInfo createInfo)
{
	this->m_VulkanDevice = createInfo.vulkanDevice;
	this->m_CommandPool = createInfo.commandPool;
	this->m_Queue = createInfo.queue;

	this->defaultAllocationSize = createInfo.defaultAllocationSize;
	this->memoryAlignment = createInfo.memoryAlignment;
	this->usage = createInfo.usage;
	this->memoryProperties = createInfo.memoryProperties;

	m_VulkanDevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		defaultAllocationSize,
		&m_StagingBuffer,
		&m_StagingBufferMemory
	);

	createBFA();
	
}

ng::memory::VulkanBuffer ng::memory::vma::VulkanMemoryAllocator::createBuffer(VkDeviceSize size)
{
	VkDeviceSize allocSize = size + memoryAlignment - (size % memoryAlignment);
	
	for (int i = 0; i < m_BufferRegionAllocators.size() - 1; ++i) {
		bool someSuitable = false;

		std::pair<VkDeviceSize, VkDeviceSize> bestFitFreeSpace;
		std::pair<VkDeviceSize, VkDeviceSize> tmpFreeSpace;

		VkDeviceSize currentSpaceDifference;
		int suitableBufferRegionIndex = 0;
		bestFitFreeSpace = m_BufferRegionAllocators[0]->findSuitableFreeSpace(allocSize);
		if (bestFitFreeSpace.second != 0) {
			someSuitable = true;
		}
		currentSpaceDifference = bestFitFreeSpace.second - allocSize;

		for (int i = 1; i < m_BufferRegionAllocators.size(); ++i) {
			tmpFreeSpace = m_BufferRegionAllocators[i]->findSuitableFreeSpace(allocSize);
			if (tmpFreeSpace.second != 0) {
				someSuitable = true;
			}
			if (currentSpaceDifference >(allocSize - tmpFreeSpace.second)) {
				bestFitFreeSpace = tmpFreeSpace;
				currentSpaceDifference = allocSize - tmpFreeSpace.second;
				suitableBufferRegionIndex = i;
			}
		}

		if (!someSuitable) {
			if (createBFA() != VK_SUCCESS) { //out of memory, defragment...
				defragment();
			}
			continue;
		}

		return m_BufferRegionAllocators[suitableBufferRegionIndex]->createBuffer(bestFitFreeSpace.first, bestFitFreeSpace.second, allocSize);
	}

	LOGI("could not create buffer, fatal error!!");
	LOGD("-------||-------");
}

void ng::memory::vma::VulkanMemoryAllocator::defragment(uint32 defragmentNum = UINT32_MAX)
{
	for (int i = 0; (i < m_BufferRegionAllocators.size()) && (i < defragmentNum); ++i) {
		uint32 highestCountIndex = 0;
		for (int j = 0; j < m_BufferRegionAllocators.size() - 1; ++j) {
			if (m_BufferRegionAllocators[j+1]->getFreeSpaceCount() > m_BufferRegionAllocators[j]->getFreeSpaceCount()) {
				highestCountIndex = j+1;
			}
		}

		m_BufferRegionAllocators[highestCountIndex]->defragment();

	}
}
