#pragma once

#include "../def.h"

namespace ng {
	namespace memory {

		namespace vma {
			class VulkanBufferRegionAllocator;

		}

		struct VulkanBufferCreateInfo {
			VkDeviceSize offset;
			VkDeviceSize size;
			VkBuffer* vkBuffer;
			vma::VulkanBufferRegionAllocator* bufferRegionAllocator;
		};

		class VulkanBuffer {
		private:
			vma::VulkanBufferRegionAllocator* m_BufferRegionAllocator;
		protected:
			friend class ng::memory::vma::VulkanBufferRegionAllocator;

			void(*onUpdate)() = nullptr;

			void* m_Data = nullptr;

			VkBuffer* m_VkBuffer;

			VkDeviceSize m_Offset;

			VkDeviceSize m_Size;

		protected:

			void update();

		public:

			VulkanBuffer();

			VulkanBuffer(VulkanBufferCreateInfo createInfo);

			~VulkanBuffer();

			VulkanBuffer operator=(const VulkanBuffer& buffer);

			void setBufferUpdateCallbackFunc(void(*callbackFunc)());

			void free();

		};

	}
}