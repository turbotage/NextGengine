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

			ng::math::Vec3f max; // +x, +y, +z
			ng::math::Vec3f min; // -x, -y, -z

		public:

			AABB();

			AABB(ng::math::Vec3f* position, ng::math::Vec3f max, ng::math::Vec3f min);

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

			ng::math::Vec3f getPoint0();
			ng::math::Vec3f getPoint1();
			ng::math::Vec3f getPoint2();
			ng::math::Vec3f getPoint3();
			ng::math::Vec3f getPoint4();
			ng::math::Vec3f getPoint5();
			ng::math::Vec3f getPoint6();
			ng::math::Vec3f getPoint7();

		};
	}
}


