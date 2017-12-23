#pragma once

#include "aabb.h"
#include "../Math/vec3.h"

namespace ng {
	namespace bvolumes {
		/*One AaBb plus a radius, combined makes one AaBb plus a bounding-sphere*/
		class GeneralBV
		{
		public:

			AaBb aabb;
						 //bounding sphere
			float radius;

			GeneralBV();
			~GeneralBV();
		};
	}
}

