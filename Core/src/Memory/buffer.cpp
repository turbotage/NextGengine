#include "buffer.h"

namespace ng {
	namespace memory {

		void ng::memory::Buffer::update()
		{
		}

		ng::memory::Buffer::Buffer()
			: m_BufferCopies(0)
		{

		}

		ng::memory::Buffer::Buffer(VkDeviceSize offset, VkDeviceSize size, VkBuffer * buffer)
			: m_Offset(offset), m_Size(size), m_Buffer(buffer)
		{
			m_Data = malloc(size);
		}

		ng::memory::Buffer::Buffer(Buffer & buf)
		{
			buf.m_BufferCopies++;
			m_BufferCopies = buf.m_BufferCopies;
			m_Buffer = buf.m_Buffer;
			m_Data = buf.m_Buffer;
			m_Offset = buf.m_Offset;
			m_Size = buf.m_Size;
		}
	}
}