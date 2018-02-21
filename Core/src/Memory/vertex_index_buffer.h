#pragma once

#include "buffer.h"

namespace ng {
	namespace memory {
		class VertexIndexBuffer
		{
		private:

			VkDeviceSize m_IndexBufferOffset;

			Buffer* m_IndexBuffer;
			Buffer* m_VertexBuffer;

		public:
			VertexIndexBuffer();
			~VertexIndexBuffer();
		};
	}
}


