#pragma once

namespace ng {
	namespace memory {

		class Allocator
		{
		protected:
			void* m_Start;
			size_t m_Size;

			size_t m_UsedMemory;
			size_t m_NumAllocations;

		public:
			Allocator(size_t size, void* start);
			~Allocator();
		};
	}
}

