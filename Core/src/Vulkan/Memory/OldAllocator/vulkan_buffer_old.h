#pragma once

#include "../../def.h"

namespace ng {

	namespace vulkan {
		class VulkanDevice;
		class VulkanBufferRegionAllocator;
		class VulkanMemoryAllocator;
	}
}

namespace ng {
	namespace vulkan {

		struct VulkanBufferCreateInfo {
			/* (required) size of buffer */
			VkDeviceSize size;
			/* (optional, default = nullptr) pointer to data to be written to buffer memory */
			void* data = nullptr;
			/* (required if data isn't nullptr, otherwise optional, default = 0) 
			size in bytes to be written to buffer memory, is ignored if data pointer is nullptr */
			VkDeviceSize dataSize = 0;
			/* (optional, default = nullptr) function to be called if buffer is somehow changed by allocator */
			void (*onUpdateCallback)() = nullptr;
		};

		class VulkanBuffer {
		private:

			friend class ng::vulkan::VulkanBufferRegionAllocator;
			friend class ng::vulkan::VulkanMemoryAllocator;

			struct VulkanBufferInternalCreateInfo {
				VulkanBufferRegionAllocator* bufferRegionAllocator;
				VkDeviceSize offset;
				VkDeviceSize size;
				VkBuffer vkBuffer;
				void* data;
				VkDeviceSize dataSize;
				void(*onUpdateCallback)();
			};

			VulkanBufferRegionAllocator* m_BufferRegionAllocator = nullptr;

			VkDeviceSize m_Offset;
			
			VkDeviceSize m_Size;

			VkBuffer m_VkBuffer;

			void* m_Data = nullptr;
			
			VkDeviceSize m_DataSize;

			void(*m_OnUpdateCallback)() = nullptr;

			void update(VkDeviceSize newOffset, VkDeviceSize newSize, void* newData, VkDeviceSize newDataSize);
			
			VulkanBuffer(VulkanBufferInternalCreateInfo createInfo);
			
			void init(VulkanBufferInternalCreateInfo createInfo);
			
		public:

			VulkanBuffer();

			VulkanBuffer(const VulkanBuffer& buffer);

			VulkanBuffer(VulkanBuffer &&) = delete;

			~VulkanBuffer();
			
			VulkanBuffer operator=(const VulkanBuffer& buffer);

			/* sets the callback function to call whenever the allocator issues
			and buffer-update*/
			void setUpdateCallbackFunc(void(*callbackFunc)());

			/* writes the new data by defragmentation, OBS! will change all instances of this buffer */
			//void writeAll(void* data, VkDeviceSize nBytes, VkCommandBuffer defragBuffer);

			/* writes the new data by a single copy, OBS! will change all instances of this buffer */
			void write(void* data, VkDeviceSize dataSize);

			void free();
			
			VkBuffer getBuffer();

			VkDeviceSize getOffset();

			VkDeviceSize getDataSize();

			VkDeviceSize getBufferSize();

			static std::size_t hash(VulkanBuffer const& buffer);
			
		};

	}
}

namespace std {
	template<> struct hash<ng::vulkan::VulkanBuffer> {
		size_t operator()(ng::vulkan::VulkanBuffer const& buffer) const {
			return ng::vulkan::VulkanBuffer::hash(buffer);
		}
	};
}