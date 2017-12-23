#pragma once

#include "../Math/vec3.h"

namespace ng {
	namespace bvolumes {
		class BoundingSphere
		{
		private:
			ng::math::Vec3 m_Center;
			float m_Radius;
		public:
			BoundingSphere();
			BoundingSphere(ng::math::Vec3 pos, float radius);
		};
	}
}

