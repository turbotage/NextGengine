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
			VulkanDevice* vulkanDevice;
			VkDeviceSize defaultAllocationSize;
			VkMemoryAlignment memoryAlignment;
			VkBufferUsageFlags usage;
			VkMemoryPropertyFlags memoryProperties;
		};

		class VulkanMemoryAllocator {
		private:

			VulkanDevice* m_VulkanDevice = nullptr;

			std::vector<VulkanBufferRegionAllocator*> m_BufferRegionAllocators;

			/*  create new VulkanBufferRegionAllocator  */
			VkResult createVBRA();

			VkDeviceSize m_DefaultAllocationSize;

			VkMemoryAlignment m_MemoryAlignment;

			VkBufferUsageFlags m_Usage;

			VkMemoryPropertyFlags m_MemoryProperties;

			std::unique_lock<std::mutex> m_Lock; 

		public:

			VulkanMemoryAllocator();

			VulkanMemoryAllocator(VulkanMemoryAllocatorCreateInfo createInfo);

			VulkanMemoryAllocator(const VulkanMemoryAllocator &) = delete;
			VulkanMemoryAllocator(VulkanMemoryAllocator &&) = delete;

			~VulkanMemoryAllocator();

			void init(VulkanMemoryAllocatorCreateInfo createInfo);

			VkResult createBuffer(VulkanBuffer* vulkanBuffer, VulkanBufferCreateInfo createInfo);

			void defragment(uint32 defragmentNum = UINT32_MAX);

		};
	}
}
