#pragma once

#include "vulkan_buffer_region_allocator.h"

namespace ng {
	namespace vulkan {
		class VulkanDevice;
	}
}

namespace ng {
	namespace vulkan {

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

			graphics::VulkanDevice* VulkanDevice;

			VkCommandPool CommandPool;

			VkQueue Queue;

			std::mutex AllocatorMutex;

			VkBuffer StagingBuffer;
			VkDeviceMemory StagingBufferMemory;

			std::vector<VulkanBufferRegionAllocator*> BufferRegionAllocators;

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
