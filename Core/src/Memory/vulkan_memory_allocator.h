#pragma once

#include "vulkan_buffer_region_allocator.h"

namespace ng {
	namespace graphics {
		class VulkanDevice;
	}
}

namespace ng {
	namespace memory {
		namespace vma {

			struct VulkanMemoryAllocatorCreateInfo {
				graphics::VulkanDevice* vulkanDevice;
				VkCommandPool commandPool;
				VkQueue queue;
				VkDeviceSize defaultAllocationSize;
				VkMemoryAlignment memoryAlignment;
				VkBufferUsageFlags usage;
				VkMemoryPropertyFlags memoryProperties;
			};

			class VulkanMemoryAllocator {
			private:

				graphics::VulkanDevice* m_VulkanDevice;

				VkCommandPool m_CommandPool;

				VkQueue m_Queue;

				std::mutex m_AllocatorMutex;

				VkBuffer m_StagingBuffer;
				VkDeviceMemory m_StagingBufferMemory;

				std::vector<VulkanBufferRegionAllocator*> m_BufferRegionAllocators;

				VkResult createBFA();

			public:

				VkDeviceSize defaultAllocationSize;

				VkMemoryAlignment memoryAlignment;

				VkBufferUsageFlags usage;

				VkMemoryPropertyFlags memoryProperties;

			public:

				VulkanMemoryAllocator();

				VulkanMemoryAllocator(VulkanMemoryAllocatorCreateInfo createInfo);

				~VulkanMemoryAllocator();

				void init(VulkanMemoryAllocatorCreateInfo createInfo);

				VulkanBuffer createBuffer(VkDeviceSize size);

				void defragment(uint32 defragmentNum = UINT32_MAX);

			};
		}
	}
}
