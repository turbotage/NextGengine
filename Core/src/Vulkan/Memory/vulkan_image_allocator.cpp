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

}

