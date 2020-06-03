#include "vulkan_allocator.h"

ngv::VulkanMemoryPage::VulkanMemoryPage(std::shared_ptr<vk::UniqueDeviceMemory> pMemory, vk::DeviceSize size)
{
	m_pMemory = pMemory;
	m_Size = size;
	m_Allocator = std::make_shared<ng::AbstractFreeListAllocator>(size);

}
