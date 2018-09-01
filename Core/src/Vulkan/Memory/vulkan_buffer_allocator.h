#pragma once

#include "../../def.h"
#include "vulkan_buffer.h"
#include "vulkan_buffer_chunk.h"

namespace ng {
	namespace vulkan {


		/*
		void createBuffer(VulkanBufferCreateInfo createInfo, VulkanBuffer& buffer) :
		copy the data from createInfo.data to allocation in staging buffer,
		*/
		class VulkanBufferAllocator
		{
		private:

			friend class VulkanBuffer;
		private:

			VulkanDevice* m_VulkanDevice;

			VkDeviceSize m_StandardChunkSize;
			VkMemoryPropertyFlags m_MemoryFlags;
			VkMemoryAlignment m_MemoryAlignment;
			VkBufferUsageFlags m_BufferUsage;

			/*  When buffers should reside in device-local memory when used this are the vulkan-memory-chunks that holds the buffers for staging,
			the data will always be available here in the staging chunks to enable fast swapping. If the buffers should be non device-local, perhaps */
			std::list<VulkanBufferChunk> m_StagingChunks;

			/*  The vulkan-memory-chunks holding the buffers when they reside in device-local memory,
			never used for host visible only buffers  */
			std::list<VulkanBufferChunk> m_DeviceChunks;

			std::list<VulkanBufferChunk>::iterator addChunk(std::list<VulkanBufferChunk>* chunks, VkResult* result = nullptr);

			//VkDeviceSize getAlignedSize(VkDeviceSize size);

			/* DeviceMemory : { Heap 0, MemoryType 0 }, StagingMemory : { Heap 2, MemoryType 2 } */
			//VkResult createDeviceLocal(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer);

			/* DeviceMemory : { Heap 1, MemoryType 1 }, StagingMemory : { Heap 2, MemoryType 2 },
			OBS! Does the same thing as createDeviceLocalWithStaging */
			//VkResult createMappableDeviceLocal(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer);



		public:

			VulkanBufferAllocator(VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkMemoryAlignment alignment, VkDeviceSize standardAllocSize);

			VulkanBufferAllocator(const VulkanBufferAllocator& other) = delete;

			VulkanBufferAllocator(VulkanBuffer &&) = delete;

			void createBuffer(VulkanBufferCreateInfo createInfo, VulkanBuffer* buffer);

			void defragmentDeviceMem(uint16 chunksDefragNum = UINT16_MAX, bool waitUntilComplete = true);

			void defragmentStagingMem(uint16 chunksDefragNum = UINT16_MAX, bool waitUntilComplete = true);

		};


	}
}