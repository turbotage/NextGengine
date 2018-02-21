#pragma once

#include "../def.h"

namespace ng {
	namespace memory {

		namespace vma {
			class VkBufferRegionAllocator;
		}

		class Buffer {
		protected:
			friend class ng::memory::vma::VkBufferRegionAllocator;

			uint16 m_BufferCopies;

			VkBuffer* m_Buffer;

			void* m_Data;

			VkDeviceSize m_Offset;

			VkDeviceSize m_Size;

		protected:

			virtual void update();

		public:

			Buffer();

			Buffer(VkDeviceSize offset, VkDeviceSize size, VkBuffer* buffer);

			Buffer(Buffer &buf);

		};
	}
}