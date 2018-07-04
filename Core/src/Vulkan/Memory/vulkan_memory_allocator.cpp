#include "vulkan_memory_allocator.h"

ng::vulkan::ng::vulkan::VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanDevice * vulkanDevice)
	: m_VulkanDevice(vulkanDevice),
	m_DeviceLocalBufferAllocator(vulkanDevice, m_DefaultDeviceLocalHostVisibleMemFlags, m_DefaultDeviceLocalHostVisibleMemAlignment, m_DefaultDeviceLocalAllocSize),
	m_DeviceLocalHostVisibleBufferAllocator(vulkanDevice, m_DefaultDeviceLocalHostVisibleMemFlags, m_DefaultDeviceLocalHostVisibleMemAlignment, m_DefaultDeviceLocalHostVisibleAllocSize),
	m_DeviceLocalImageAllocator(vulkanDevice, m_DefaultDeviceLocalHostVisibleMemFlags, m_DefaultDeviceLocalHostVisibleMemAlignment, m_DefaultDeviceLocalAllocSize),
	m_DeviceLocalHostVisibleImageAllocator(vulkanDevice, m_DefaultDeviceLocalHostVisibleMemFlags, m_DefaultDeviceLocalHostVisibleMemAlignment, m_DefaultDeviceLocalAllocSize)
{

}

VkResult ng::vulkan::ng::vulkan::VulkanMemoryAllocator::createBuffer(VulkanBuffer * buffer, VulkanBufferCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType)
{
	if (memoryType == VMA_DEVICE_LOCAL_ONLY) {
		m_DeviceLocalBufferAllocator.createBuffer(createInfo, buffer);
	}
	else if (memoryType == VMA_DEVICE_LOCAL_HOST_VISIBLE){
		m_DeviceLocalHostVisibleBufferAllocator.createBuffer(createInfo, buffer);
	}
}

VkResult ng::vulkan::ng::vulkan::VulkanMemoryAllocator::createImage(VulkanImage * image, VulkanImageCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType)
{
	if (memoryType == VMA_DEVICE_LOCAL_ONLY) {
		
	}
	else if (memoryType == VMA_DEVICE_LOCAL_HOST_VISIBLE) {

	}
}
