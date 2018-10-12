#pragma once

#include "plane.h"
#include "quaternion.h"

namespace ng {

	struct CameraCreateInfo {
		float fov;
		float viewDistance;
		float htwRatio;
		/**  the translation that should be applied to the camera from it's start position (0,0,0)  */
		ngm::Vec3f startTranslation;
		/**  the rotation that should be applied to the camera from it's start rotation (0,0,0) <-- in euler angles */
		ngm::Quaternion startRotation;
	};

	class Camera
	{
	private:
		bool isInView();

	public:

		float fieldOfView;

		float farPlaneDistance;
		float nearPlaneDistance;
		float halfWidth;
		float halfHeight;

		ngm::Quaternion rotation;
		ngm::Vec3f position;

		ngm::Plane rightPlane;
		ngm::Plane leftPlane;
		ngm::Plane topPlane;
		ngm::Plane bottomPlane;

		ngm::Plane nearPlane;
		ngm::Plane farPlane;
		ngm::Plane cullPlane1;
		ngm::Plane cullPlane2;

		ngm::Vec3f forward;
		ngm::Vec3f up;
		ngm::Vec3f right;

		ngm::Vec3f rightPlaneNormal;
		ngm::Vec3f leftPlaneNormal;
		ngm::Vec3f topPlaneNormal;
		ngm::Vec3f bottomPlaneNormal;

		Camera();
		Camera(CameraCreateInfo createInfo);

		void init(CameraCreateInfo createInfo);

		~Camera();

		void update(ngm::Vec3f positionChange, ngm::Quaternion rotationChange);

	};
}

