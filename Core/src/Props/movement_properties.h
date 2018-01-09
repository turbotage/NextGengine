#pragma once

#include "../Math/vec3f.h"

namespace ng {
	namespace props {

		typedef int mMovementPropertiesMask;

		enum eMovementPropertyBit {
			MOVEMENT_ENABLED_BIT = 0x02,
			POINT_ROTATION_BIT = 0x04,
			LOCAL_ROTATION_BIT = 0x08,
			LINEAR_MOVEMENT_BIT = 0x10,
			VELOCITY_BIT = 0x20,
			ACCELERATION_BIT = 0x40,
			JERK_BIT = 0x80
		};

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


