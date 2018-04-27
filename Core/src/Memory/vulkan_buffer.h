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
			vma::VulkanBufferRegionAllocator* m_BufferRegionAllocator = nullptr;
		//protected:
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

			void init(VulkanBufferCreateInfo createInfo);

			~VulkanBuffer();

			VulkanBuffer operator=(const VulkanBuffer& buffer);

			void setBufferUpdateCallbackFunc(void(*callbackFunc)());

			void free();

			static std::size_t hash(VulkanBuffer const& buffer);

		};

	}
}

namespace std {
	template<> struct hash<ng::memory::VulkanBuffer> {
		size_t operator()(ng::memory::VulkanBuffer const& buffer) const {
			return ng::memory::VulkanBuffer::hash(buffer);
		}
	};
}