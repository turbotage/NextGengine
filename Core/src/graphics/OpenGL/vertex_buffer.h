#pragma once

#include "../def.h"
#include "graphics.h"

namespace ng {
	namespace graphics {
		class VertexBuffer
		{
		private:
			uint m_Handle;
			uint m_Size;
			GLenum m_Usage;
		public:

			VertexBuffer();
			~VertexBuffer();

			void resize(uint size);
			void setData(uint size, const void* data);

			void bind();
			void unbind();
			void release();

		};
	}
}

