#pragma once

#include "../../def.h"

namespace ng {

	namespace vulkan {
		class VulkanDevice;
		class VulkanBufferRegionAllocator;
	}
}

namespace ng {
	namespace vulkan {

		struct VulkanBufferCreateInfo {
			VulkanBufferRegionAllocator* bufferRegionAllocator;
			VkDeviceSize offset;
			VkDeviceSize size;
			VkDeviceSize dataSize;
			VkBuffer* vkBuffer;
			void* data;
			void(*callback)();
		};

		class VulkanBuffer {
		private:
			VulkanBufferRegionAllocator* m_BufferRegionAllocator = nullptr;
		//protected:
			friend class ng::vulkan::VulkanBufferRegionAllocator;

			VkDeviceSize m_Offset;
			
			VkDeviceSize m_Size;

			VkDeviceSize m_DataSize;

			VkBuffer* m_VkBuffer;

			void* m_Data = nullptr;

			void(*m_OnUpdate)() = nullptr;
			

		protected:

			void update();
			
			VulkanBuffer(VulkanBufferCreateInfo createInfo);
			
			void init(VulkanBufferCreateInfo createInfo);
			
		public:

			VulkanBuffer();

			VulkanBuffer(const VulkanBuffer& buffer);

			~VulkanBuffer();
			
			VulkanBuffer operator=(const VulkanBuffer& buffer);

			/* sets the callback function to call whenever the allocator issues
			and buffer-update*/
			void setBufferUpdateCallbackFunc(void(*callbackFunc)());

			/* writes the new data by defragmentation, OBS! will change all instances of this buffer */
			//void writeAll(void* data, VkDeviceSize nBytes, VkCommandBuffer defragBuffer);

			/* writes the new data by a single copy, OBS! will change all instances of this buffer */
			void write(void* data, VkDeviceSize nBytes, VkCommandBuffer writeBuffer);

			void free();

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