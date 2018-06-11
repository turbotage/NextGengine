
#include "../vulkan_device.h"
#include "../../debug.h"
#include "vulkan_memory_allocator.h"
#include "vulkan_buffer.h"
#include "../Threading/vulkan_synchronisation.h"

#define DEFAULT_ALLOC_SIZE 536870912

VkResult ng::vulkan::VulkanMemoryAllocator::createVBRA()
{
	VulkanBufferRegionAllocatorCreateInfo bfaCreateInfo;
	bfaCreateInfo.vulkanDevice = m_VulkanDevice;
	bfaCreateInfo.memorySize = m_DefaultAllocationSize;
	bfaCreateInfo.memoryAlignment = m_MemoryAlignment;

	VulkanBufferRegionAllocator* bfa = new VulkanBufferRegionAllocator(bfaCreateInfo);
	VkResult res = m_VulkanDevice->createBuffer(m_Usage, m_MemoryProperties, m_DefaultAllocationSize, &bfa->m_VkBuffer, &bfa->m_VkBufferMemory);
	if (res != VK_SUCCESS) {
		delete bfa;
	}
	else {
		m_BufferRegionAllocators.push_back(bfa);
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
	this->m_VulkanDevice = createInfo.vulkanDevice;

	this->m_DefaultAllocationSize = createInfo.defaultAllocationSize;
	this->m_MemoryAlignment = createInfo.memoryAlignment;
	this->m_Usage = createInfo.usage;
	this->m_MemoryProperties = createInfo.memoryProperties;

	createVBRA();
	
}

VkResult ng::vulkan::VulkanMemoryAllocator::createBuffer(VulkanBuffer* vulkanBuffer, VulkanBufferCreateInfo createInfo)
{
	VkDeviceSize allocSize = createInfo.size + m_MemoryAlignment - (createInfo.size % m_MemoryAlignment);
	
	assert(allocSize <= m_DefaultAllocationSize);

	createInfo.size = allocSize;

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
			if (currentSpaceDifference > (allocSize - tmpFreeSpace.second)) {
				bestFitFreeSpace = tmpFreeSpace;
				currentSpaceDifference = allocSize - tmpFreeSpace.second;
				suitableBufferRegionIndex = i;
			}
		}

		if (!someSuitable) {
			VkResult res = createVBRA();
			if (res != VK_SUCCESS) { //out of memory, defragment...
				return res;
			}
			continue;
		}

		m_BufferRegionAllocators[suitableBufferRegionIndex]->createBuffer(vulkanBuffer, bestFitFreeSpace.first, bestFitFreeSpace.second, createInfo);
	}

	LOGI("could not create buffer, fatal error!!");
	LOGD("-------||-------");
}

void ng::vulkan::VulkanMemoryAllocator::defragment(uint32 defragmentNum = UINT32_MAX)
{
	VkCommandBuffer commandBuffer = m_VulkanDevice->createCommandBuffer(
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		m_VulkanDevice->memoryCommandPool,
		true
	);

	bool isLocked = false;

	for (int i = 0; (i < m_BufferRegionAllocators.size()) && (i < defragmentNum); ++i) {
		uint32 highestCountIndex = 0;
		for (int j = 0; j < m_BufferRegionAllocators.size() - 1; ++j) {
			if (m_BufferRegionAllocators[j+1]->getFreeSpaceCount() > m_BufferRegionAllocators[j]->getFreeSpaceCount()) {
				highestCountIndex = j+1;
			}
		}

		if (!isLocked) {
			isLocked = true;
			m_BufferRegionAllocators[highestCountIndex]->defragment(commandBuffer, &m_Lock);
		}
		else {
			m_BufferRegionAllocators[highestCountIndex]->defragment(commandBuffer, nullptr);
		}
	}

	vkQueueWaitIdle(m_VulkanDevice->computeQueue);
	vkQueueWaitIdle(m_VulkanDevice->graphicsQueue);

	m_VulkanDevice->flushCommandBuffer(commandBuffer, m_VulkanDevice->memoryCommandPool, m_VulkanDevice->transferQueue, true);
	if (isLocked) {
		m_Lock.unlock();
	}
}
