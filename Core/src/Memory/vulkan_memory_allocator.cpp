#include "../Graphics/vulkan_device.h"
#include "vulkan_memory_allocator.h"

#define DEFAULT_ALLOC_SIZE 536870912

void ng::memory::vma::VulkanMemoryAllocator::createStagingBufferAndMemory(VkDeviceSize size) {

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m, &bufferInfo, nullptr, &m_StagingBuffer)) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_VulkanDevice->logicalDevice, m_StagingBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(m_VulkanDevice->logicalDevice, &allocInfo, nullptr, &m_StagingBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory");
	}

	vkBindBufferMemory(m_VulkanDevice->logicalDevice, m_StagingBuffer, m_StagingBufferMemory, 0);
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

	VulkanBufferRegionAllocatorCreateInfo bfaCreateInfo;
	bfaCreateInfo.vulkanDevice = m_VulkanDevice;
	bfaCreateInfo.stagingBuffer = m_StagingBuffer;
	bfaCreateInfo.stagingBufferMemory = m_StagingBufferMemory;
	bfaCreateInfo.memorySize = defaultAllocationSize;
	bfaCreateInfo.memoryAlignment = memoryAlignment;
	bfaCreateInfo.commandPool = m_CommandPool;
	bfaCreateInfo.queue = m_Queue;


	VulkanBufferRegionAllocator* bfa = new VulkanBufferRegionAllocator(bfaCreateInfo);
	m_VulkanDevice->createBuffer(usage, memoryProperties, defaultAllocationSize, &bfa->buffer, &bfa->bufferMemory);

	m_BufferRegionAllocators.push_back(bfa);
	
}

void ng::memory::vma::VulkanMemoryAllocator::createVkBufferAndMemory(
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

uint32 ng::memory::vma::VulkanMemoryAllocator::findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
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

ng::memory::VulkanBuffer ng::memory::vma::VulkanMemoryAllocator::createBuffer(VkDeviceSize size)
{
	VkDeviceSize allocSize = size + memoryAlignment - (size % memoryAlignment);
	
	std::pair<VkDeviceSize, VkDeviceSize> bestFitFreeSpace;
	std::pair<VkDeviceSize, VkDeviceSize> tmpFreeSpace;

	VkDeviceSize currentSpaceDifference;
	int suitableBufferRegionIndex = 0;
	bestFitFreeSpace = m_BufferRegionAllocators[0]->findSuitableFreeSpace(allocSize);
	currentSpaceDifference = bestFitFreeSpace.second - allocSize;

	for (int i = 1; i < m_BufferRegionAllocators.size(); ++i) {
		tmpFreeSpace = m_BufferRegionAllocators[i]->findSuitableFreeSpace(allocSize);
		if (currentSpaceDifference > (allocSize - tmpFreeSpace.second)) {
			bestFitFreeSpace = tmpFreeSpace;
			currentSpaceDifference = allocSize - tmpFreeSpace.second;
			suitableBufferRegionIndex = i;
		}
	}

	return m_BufferRegionAllocators[suitableBufferRegionIndex]->createBuffer(bestFitFreeSpace.first, bestFitFreeSpace.second, allocSize);
}

void ng::memory::vma::VulkanMemoryAllocator::defragment()
{

	for (int i = 1; i < m_BufferRegionAllocators.size(); ++i) {

		

	}

}
