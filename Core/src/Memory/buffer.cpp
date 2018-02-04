#include "buffer.h"

namespace ng {
	namespace vma {
		ng::vma::Buffer::Buffer()
			: m_BufferCopies(0)
		{

		}

		ng::vma::Buffer::Buffer(VkDeviceSize offset, VkDeviceSize size, VkBuffer * buffer)
			: m_Offset(offset), m_Size(size), m_Buffer(buffer)
		{
			m_Data = malloc(size);
		}

		ng::vma::Buffer::Buffer(Buffer & buf)
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