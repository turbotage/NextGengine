#pragma once

#include "../def.h"
#include <smmintrin.h>

namespace ng {
	namespace math {
		class Quaternion
		{
		private:
			union {
				ALIGN(16) float x, y, z, w;
				__m128 row;
			};
		public:
			Quaternion();
			~Quaternion();
		};
	}
}
