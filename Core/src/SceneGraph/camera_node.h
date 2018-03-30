#pragma once

#include "scene_node.h"
#include "../Math/plane.h"

namespace ng {
	namespace scenegraph {

		class CameraNode : SceneNode
		{
		private:

			

		public:

			const float fov;

			const float farClipDistance;
			float nearClipDistance;
			float halfWidth;
			float halfHeight;

			ng::math::Vec3f forward;
			ng::math::Vec3f up;
			ng::math::Vec3f right;

			ng::math::Vec3f rightPlaneNormal;
			ng::math::Vec3f leftPlaneNormal;
			ng::math::Vec3f topPlaneNormal;
			ng::math::Vec3f bottomPlaneNormal;

			ng::math::Plane farPlane;
			ng::math::Plane nearPlane;
			ng::math::Plane rightPlane;
			ng::math::Plane leftPlane;
			ng::math::Plane topPlane;
			ng::math::Plane bottomPlane;


			CameraNode(float fov, float viewDistance, float htwRatio);

			void onUpdate(float time) override;

			~CameraNode();
		};

	}
}
