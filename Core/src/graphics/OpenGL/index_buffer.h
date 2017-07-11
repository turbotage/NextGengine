#pragma once

#include "../def.h"

#include "graphics.h"

namespace ng {
	namespace graphics {
		class IndexBuffer
		{
		private:
			uint m_Handle;
			uint m_Count;
		public:
			IndexBuffer(uint16* data, uint count);
			IndexBuffer(uint32* data, uint count);
			~IndexBuffer();
			
			void bind();
			void unbind();

			uint getCount();
		};
	}
}

