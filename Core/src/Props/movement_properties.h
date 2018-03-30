#pragma once

#include "../Math/vec3f.h"

namespace ng {
	namespace props {

		struct PointRotation {
			ng::math::Vec3f rotationPoint;
			ng::math::Vec3f rotationAxis;
			float angularVelocity;
			float angularAcceleration;
			float angularJerk;
		};

		struct LocalRotation {
			ng::math::Vec3f rotationAxis;
			float angularVelocity;
			float angularAcceleration;
			float angularJerk;
		};

		typedef ng::math::Vec3f Velocity;

		typedef ng::math::Vec3f Acceleration;

		typedef ng::math::Vec3f Jerk;

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


