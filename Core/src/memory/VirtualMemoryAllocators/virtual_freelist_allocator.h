#pragma once

#include "../../def.h"

namespace ng {
	namespace memory {

		class VirtualFreeListAllocator
		{
		public:

			uint64 addressSpace;

			uint64 allocate(size_t bytes);

			bool deallocate();
		};

	}
}
