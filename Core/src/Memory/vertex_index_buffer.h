#pragma once

#include "buffer.h"

namespace ng {
	namespace memory {
		class VertexIndexBuffer
		{
		private:

			VkDeviceSize m_IndexBufferOffset;

		public:

			VertexIndexBuffer();
			~VertexIndexBuffer();
		};
	}
}


