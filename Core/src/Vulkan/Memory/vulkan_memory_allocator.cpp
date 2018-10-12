#include "vulkan_memory_allocator.h"
#include "vulkan_buffer.h"

ng::vulkan::ng::vulkan::VulkanMemoryAllocator::VulkanMemoryAllocator()
{

}

ng::vulkan::ng::vulkan::VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocatorCreateInfo createInfo)
{
	create(createInfo);
}

void ng::vulkan::ng::vulkan::VulkanMemoryAllocator::create(VulkanMemoryAllocatorCreateInfo createInfo)
{
	if (hasBeenCreated) {
		return;
	}

	m_VulkanDevice = createInfo.vulkanDevice;

	VulkanBufferAllocatorCreateInfo bufferAllocatorInfo;
	bufferAllocatorInfo.vulkanDevice = m_VulkanDevice;
	bufferAllocatorInfo.memoryFlags = m_DefaultDeviceLocalMemFlags;
	bufferAllocatorInfo.alignment = m_DefaultDeviceLocalMemAlignment;
	bufferAllocatorInfo.standardChunkSize = m_DefaultDeviceLocalAllocSize;

	m_DeviceLocalBufferAllocator.create(bufferAllocatorInfo);

	//vulkanDevice is same as earlier, need not be set
	bufferAllocatorInfo.memoryFlags = m_DefaultDeviceLocalHostVisibleMemFlags;
	bufferAllocatorInfo.alignment = m_DefaultDeviceLocalHostVisibleMemAlignment;
	bufferAllocatorInfo.standardChunkSize = m_DefaultDeviceLocalHostVisibleAllocSize;

	m_DeviceLocalBufferAllocator.create(bufferAllocatorInfo);

	VulkanImageAllocatorCreateInfo imageAllocatorInfo;
	imageAllocatorInfo.vulkanDevice = m_VulkanDevice;

	m_DeviceLocalImageAllocator.create(imageAllocatorInfo);

	m_DeviceLocalHostVisibleImageAllocator.create(imageAllocatorInfo);
	
}

VkResult ng::vulkan::ng::vulkan::VulkanMemoryAllocator::createBuffer(VulkanBufferCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType, VulkanBuffer * buffer)
{
	if (memoryType == VMA_DEVICE_LOCAL_ONLY) {
		m_DeviceLocalBufferAllocator.createBuffer(createInfo, buffer);
	}
	else if (memoryType == VMA_DEVICE_LOCAL_HOST_VISIBLE){
		m_DeviceLocalHostVisibleBufferAllocator.createBuffer(createInfo, buffer);
	}

	return VK_SUCCESS;
}

VkResult ng::vulkan::ng::vulkan::VulkanMemoryAllocator::createTexture2D(VulkanImageCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType, VulkanTexture2D * image)
{
	if (memoryType == VMA_DEVICE_LOCAL_ONLY) {
		m_DeviceLocalImageAllocator.createTexture2D(createInfo, image);
	}
	else if (memoryType == VMA_DEVICE_LOCAL_HOST_VISIBLE) {
		m_DeviceLocalHostVisibleImageAllocator.createTexture2D(createInfo, image);
	}

	return VK_SUCCESS;
}

VkResult ng::vulkan::ng::vulkan::VulkanMemoryAllocator::createTextureArray(VulkanImageCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType, VulkanTextureArray * image)
{
	if (memoryType == VMA_DEVICE_LOCAL_ONLY) {
		m_DeviceLocalImageAllocator.createTextureArray(createInfo, image);
	}
	else if (memoryType == VMA_DEVICE_LOCAL_HOST_VISIBLE) {
		m_DeviceLocalHostVisibleImageAllocator.createTextureArray(createInfo, image);
	}

	return VK_SUCCESS;
}
