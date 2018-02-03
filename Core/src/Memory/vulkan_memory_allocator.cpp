#include "vulkan_memory_allocator.h"

#define DEFAULT_ALLOC_SIZE 536870912

void ng::vma::VulkanMemoryAllocator::createStagingBufferAndMemory(VkDeviceSize size) {

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(*m_Device, &bufferInfo, nullptr, &m_StagingBuffer)) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*m_Device, m_StagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(*m_Device, &allocInfo, nullptr, &m_StagingBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory");
	}

	vkBindBufferMemory(*m_Device, m_StagingBuffer, m_StagingBufferMemory, 0);
}

ng::vma::VulkanMemoryAllocator::VulkanMemoryAllocator()
{

}

void ng::vma::VulkanMemoryAllocator::init(VulkanMemoryAllocatorCreateInfo createInfo)
{
	m_CreateInfo = createInfo;
	createStagingBufferAndMemory(createInfo.defaultAllocationSize);
	m_BufferRegionAllocators.emplace_back(createInfo.defaultAllocationSize, createInfo.memoryAlignment);
	createVkBufferAndMemory(
		&m_BufferRegionAllocators.end()->buffer,
		&m_BufferRegionAllocators.end()->bufferMemory,
		createInfo.defaultAllocationSize,
		createInfo.usage,
		createInfo.properties,
		createInfo.sharingMode
	);
}

void ng::vma::VulkanMemoryAllocator::createVkBufferAndMemory(
	VkBuffer* buffer,
	VkDeviceMemory* memory,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkSharingMode sharingMode)
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = sharingMode;

	if (vkCreateBuffer(*m_Device, &createInfo, nullptr, buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*m_Device, m_StagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(*m_Device, &allocInfo, nullptr, &m_StagingBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory");
	}

	vkBindBufferMemory(*m_Device, m_StagingBuffer, m_StagingBufferMemory, 0);
}

uint32 ng::vma::VulkanMemoryAllocator::findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(*m_PhysicalDevice, &memProperties);
	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}

ng::vma::Buffer* ng::vma::VulkanMemoryAllocator::createBuffer(VkDeviceSize size)
{
	VkDeviceSize allocSize = size + m_CreateInfo.memoryAlignment - (size % m_CreateInfo.memoryAlignment);
	
	std::pair<VkDeviceSize, VkDeviceSize> bestFitFreeSpace;
	std::pair<VkDeviceSize, VkDeviceSize> tmpFreeSpace;

	VkDeviceSize currentSpaceDifference;
	int suitableBufferRegionIndex = 0;
	bestFitFreeSpace = m_BufferRegionAllocators[0].findSuitableFreeSpace(allocSize);
	currentSpaceDifference = bestFitFreeSpace.second - allocSize;

	for (int i = 1; i < m_BufferRegionAllocators.size(); ++i) {
		tmpFreeSpace = m_BufferRegionAllocators[i].findSuitableFreeSpace(allocSize);
		if (currentSpaceDifference > (allocSize - tmpFreeSpace.second)) {
			bestFitFreeSpace = tmpFreeSpace;
			currentSpaceDifference = allocSize - tmpFreeSpace.second;
			suitableBufferRegionIndex = i;
		}
	}

	return m_BufferRegionAllocators[suitableBufferRegionIndex].createBuffer(bestFitFreeSpace.first, bestFitFreeSpace.second);
}

void ng::vma::VulkanMemoryAllocator::freeBuffer(Buffer* buffer)
{
	
	for (const auto& bufferRegion : m_BufferRegionAllocators) {
		if (bufferRegion) {

		}
	}

}

void ng::vma::VulkanMemoryAllocator::defragment()
{

}
