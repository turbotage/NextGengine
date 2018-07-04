#pragma once

#include "../vulkan_device.h"
#include "vulkan_buffer_allocator.h"
#include "vulkan_image_allocator.h"

#define MEMORY_ALIGNMENT 16

namespace ng {
	namespace vulkan {

		namespace ng {
			namespace vulkan {

				/*TODO : maybe add priorities to the allocations, that is to say which allocations that is prefered to stay
				in device-memory, shouldn't be swapped if not absolutely neccesary	*/

				enum eVulkanMemoryAllocatorMemoryType {
					VMA_DEVICE_LOCAL_ONLY,
					VMA_DEVICE_LOCAL_HOST_VISIBLE,
				};
				 
				class VulkanMemoryAllocator {
				private:
					VulkanDevice* m_VulkanDevice;

					//device-local props
					VkMemoryPropertyFlags m_DefaultDeviceLocalMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
					VkMemoryAlignment m_DefaultDeviceLocalMemAlignment = 64;
					VkDeviceSize m_DefaultDeviceLocalAllocSize = 268435456; //256 MB

					//device-local host-visible props
					VkMemoryPropertyFlags m_DefaultDeviceLocalHostVisibleMemFlags =
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
					VkMemoryAlignment m_DefaultDeviceLocalHostVisibleMemAlignment = 64;
					VkDeviceSize m_DefaultDeviceLocalHostVisibleAllocSize = 67108864; //64 MB


					//device-local only
					VulkanBufferAllocator m_DeviceLocalBufferAllocator;

					//device-local host-visible
					VulkanBufferAllocator m_DeviceLocalHostVisibleBufferAllocator;
					
					//device-local only
					VulkanImageAllocator m_DeviceLocalImageAllocator;

					//deivce-local host-visible
					VulkanImageAllocator m_DeviceLocalHostVisibleImageAllocator;

				public:

					VulkanMemoryAllocator(VulkanDevice* vulkanDevice);

					VkResult createBuffer(VulkanBuffer* buffer, VulkanBufferCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType);

					VkResult createImage(VulkanImage* image, VulkanImageCreateInfo createInfo, eVulkanMemoryAllocatorMemoryType memoryType);

				};

			}
		}

	}
}