#pragma once

#include "../def.h"

namespace ng {
	namespace vma {
		class Buffer {
		private:
			friend class VkBufferRegionAllocator;

			uint16 m_BufferCopies;

			VkBuffer* m_Buffer;

			void* m_Data;

			VkDeviceSize m_Offset;

			VkDeviceSize m_Size;

		public:

			Buffer();

			Buffer(VkDeviceSize offset, VkDeviceSize size, VkBuffer* buffer);

			Buffer(Buffer &buf);

		};
	}
}