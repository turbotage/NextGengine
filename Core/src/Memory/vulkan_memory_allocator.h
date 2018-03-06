#pragma once

#include "vulkan_buffer_region_allocator.h"


namespace ng {
	namespace memory {
		namespace vma {

			struct VulkanMemoryAllocatorCreateInfo {
				VkDeviceSize defaultAllocationSize;
				VkMemoryAlignment memoryAlignment;
				VkBufferUsageFlags usage;
				VkMemoryPropertyFlags properties;
				VkSharingMode sharingMode;
			};

			class VulkanMemoryAllocator {
			private:

				std::mutex m_DefragmentationMutex;

				VulkanMemoryAllocatorCreateInfo m_CreateInfo;

				VkDevice* m_Device;
				VkPhysicalDevice* m_PhysicalDevice;

				VkMemoryPropertyFlags memoryProperties;

				VkBuffer m_StagingBuffer;
				VkDeviceMemory m_StagingBufferMemory;

				std::vector<VkBufferRegionAllocator> m_BufferRegionAllocators;

				void createStagingBufferAndMemory(VkDeviceSize size);

			protected:

			public:

				VulkanMemoryAllocator();

				void init(VulkanMemoryAllocatorCreateInfo createInfo);

				void createVkBufferAndMemory(
					VkBuffer* buffer,
					VkDeviceMemory* memory,
					VkDeviceSize size,
					VkBufferUsageFlags usage,
					VkMemoryPropertyFlags properties,
					VkSharingMode sharingMode);

				uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);

				VulkanBuffer createBuffer(VkDeviceSize size);

				void defragment();

			};
		}
	}
}
