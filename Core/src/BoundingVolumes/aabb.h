#pragma once

#include "../Math/Vec3f.h"

namespace ng {
	namespace bvolumes {
		class AABB
		{
		private:

			//aabb 
			//	1		0
			//	|	2	|	3
			//	|	|	|	|
			//	5	|	4	|
			//		6		7

			//is local origin, therefore smaller x is negative x and bigger is positive x, and so on for y and z aswell
			ng::math::Vec3f* position;

			float positiveWidth; // +x
			float negativeWidth; // -x

			float positiveHeight; // +y
			float negativeHeight; // -y

			float positiveDepth; // +z 
			float negativeDepth; // -z

		public:

			AABB();

			~AABB();

			/*please observe all get functions bellow returns the position in terms of the space given by the position pointer
			and all set functions bellow requireds positins relative to position, 
			i.e all gets returns the dimension + postion, and all sets set pos to the dimension from a position + dimension*/

			float maxX();
			void setMaxX(float maxX);

			float minX();
			void setMinX(float minX);

			float maxY();
			void setMaxY(float maxY);

			float minY();
			void setMinY(float minY);

			float maxZ();
			void setMaxZ(float maxZ);

			float minZ();
			void setMinZ(float minZ);

		};
	}
}


