#pragma once

#include "vec3f.h"

namespace ng {
	namespace props {

		struct PointRotation {
			ngm::Vec3f rotationPoint;
			ngm::Vec3f rotationAxis;
			float angularVelocity;
			float angularAcceleration;
			float angularJerk;
		};

		struct LocalRotation {
			ngm::Vec3f rotationAxis;
			float angularVelocity;
			float angularAcceleration;
			float angularJerk;
		};

		typedef ngm::Vec3f Velocity;

		typedef ngm::Vec3f Acceleration;

		typedef ngm::Vec3f Jerk;

		struct LinearMovement {
			Velocity velocity;
			Acceleration acceleration;
			Jerk jerk;
		};

		class MovementProperties
		{
		public:
			MovementProperties();
			~MovementProperties();
		};

	}
}


