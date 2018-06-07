
#include "../vulkan_device.h"
#include "../../debug.h"
#include "vulkan_memory_allocator.h"

#define DEFAULT_ALLOC_SIZE 536870912

VkResult ng::vulkan::VulkanMemoryAllocator::createBFA()
{
	VulkanBufferRegionAllocatorCreateInfo bfaCreateInfo;
	bfaCreateInfo.vulkanDevice = VulkanDevice;
	bfaCreateInfo.memorySize = defaultAllocationSize;
	bfaCreateInfo.memoryAlignment = memoryAlignment;

	VulkanBufferRegionAllocator* bfa = new VulkanBufferRegionAllocator(bfaCreateInfo);
	VkResult res = VulkanDevice->createBuffer(usage, memoryProperties, defaultAllocationSize, &bfa->buffer, &bfa->bufferMemory);
	if (res != VK_SUCCESS) {
		delete bfa;
	}
	else {
		BufferRegionAllocators.push_back(bfa);
	}

	return res;
}

ng::vulkan::VulkanMemoryAllocator::VulkanMemoryAllocator()
{

}

ng::vulkan::VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocatorCreateInfo createInfo)
{
	init(createInfo);
}

ng::vulkan::VulkanMemoryAllocator::~VulkanMemoryAllocator()
{

}

void ng::vulkan::VulkanMemoryAllocator::init(VulkanMemoryAllocatorCreateInfo createInfo)
{
	this->VulkanDevice = createInfo.vulkanDevice;
	this->CommandPool = createInfo.commandPool;
	this->Queue = createInfo.queue;

	this->defaultAllocationSize = createInfo.defaultAllocationSize;
	this->memoryAlignment = createInfo.memoryAlignment;
	this->usage = createInfo.usage;
	this->memoryProperties = createInfo.memoryProperties;

	VulkanDevice->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		defaultAllocationSize,
		&StagingBuffer,
		&StagingBufferMemory
	);

	createBFA();
	
}

ng::vulkan::VulkanBuffer ng::memory::vma::VulkanMemoryAllocator::createBuffer(VkDeviceSize size)
{
	VkDeviceSize allocSize = size + memoryAlignment - (size % memoryAlignment);
	
	assert(allocSize <= defaultAllocationSize);

	for (int i = 0; i < BufferRegionAllocators.size() - 1; ++i) {
		bool someSuitable = false;

		std::pair<VkDeviceSize, VkDeviceSize> bestFitFreeSpace;
		std::pair<VkDeviceSize, VkDeviceSize> tmpFreeSpace;

		VkDeviceSize currentSpaceDifference;
		int suitableBufferRegionIndex = 0;
		bestFitFreeSpace = BufferRegionAllocators[0]->findSuitableFreeSpace(allocSize);
		if (bestFitFreeSpace.second != 0) {
			someSuitable = true;
		}
		currentSpaceDifference = bestFitFreeSpace.second - allocSize;

		for (int i = 1; i < BufferRegionAllocators.size(); ++i) {
			tmpFreeSpace = BufferRegionAllocators[i]->findSuitableFreeSpace(allocSize);
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
				defragment(2);
			}
			continue;
		}

		return BufferRegionAllocators[suitableBufferRegionIndex]->createBuffer(bestFitFreeSpace.first, bestFitFreeSpace.second, allocSize);
	}

	LOGI("could not create buffer, fatal error!!");
	LOGD("-------||-------");
}

void ng::vulkan::VulkanMemoryAllocator::defragment(uint32 defragmentNum = UINT32_MAX)
{
	for (int i = 0; (i < BufferRegionAllocators.size()) && (i < defragmentNum); ++i) {
		uint32 highestCountIndex = 0;
		for (int j = 0; j < BufferRegionAllocators.size() - 1; ++j) {
			if (BufferRegionAllocators[j+1]->getFreeSpaceCount() > BufferRegionAllocators[j]->getFreeSpaceCount()) {
				highestCountIndex = j+1;
			}
		}

		BufferRegionAllocators[highestCountIndex]->defragment();

	}
}
