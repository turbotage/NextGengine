#pragma once

namespace ng {
	namespace vulkan {

		class VulkanImageAllocation {
			friend class VulkanImage;

			/* Ptr to the Staging VulkanBufferAllocation corresponding to a device allocation,
			OBS!! Only used for device-local vulkan-allocations */
			VulkanImageAllocation* m_StagingAllocation = nullptr;
		public:
			VkDeviceSize offset;
			VkDeviceSize size;
			VkMemoryAlignment memoryAlignment;
			uint32 width, height;
			uint32 mipLevels;
			uint32 layerCount;

		};

		class VulkanImageChunk {

			std::atomic<bool> hasFailedToFindMatch;
			std::atomic<VkDeviceSize> lastFailedAllocateSize;

			VkDeviceMemory memory;

			VkDeviceSize size;

			VkDeviceSize totalFreeSpace;

			std::list<Block> freeBlocks;
			std::list<VulkanImageAllocation> allocations;



			VkResult create(VulkanDevice* vulkanDevice,
				uint32 memoryTypeIndex,
				VkMemoryPropertyFlags flags);

		};

	}
}