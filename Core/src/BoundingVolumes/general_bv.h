#pragma once

#include "aabb.h"
#include "vec3f.h"

namespace ng {
	namespace bvolumes {
		/*One AaBb plus a radius, combined makes one AaBb plus a bounding-sphere*/
		class GeneralBV
		{
		public:

			AABB aabb;
						 //bounding sphere
			float radius;

			GeneralBV();
			~GeneralBV();
		};
	}
}

