#pragma once

#include "../def.h"
#include <vector>

namespace ng {
	namespace graphics {

		class VertexBuffer;

		class VertexArray
		{
		private:
			std::vector<VertexBuffer*> m_Buffers;
		public:

			VertexArray();
			~VertexArray();

			VertexBuffer* getBuffer(uint index = 0);
			void pushBuffer(VertexBuffer* buffer);

			void bind();
			void unbind();

			void draw(uint count);

		};
	}
}

