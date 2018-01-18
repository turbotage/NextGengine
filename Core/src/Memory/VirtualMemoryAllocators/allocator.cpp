#include "allocator.h"

void ng::memory::Allocator::init(uint64 size)
{	
	uint8 rest = size % MEMORY_ALIGNMENT;
	m_MemorySize = size - rest;
	m_FreeMemorySize = m_MemorySize;
	std::pair<uint64, uint64> freeSpace;
	freeSpace.second = 0;
	freeSpace.first = m_MemorySize;
	m_FreeSpaces.insert(freeSpace);
	printf("%" PRIu64 "\n", m_MemorySize);
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

	}
}
