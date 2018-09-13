#pragma once

#include "scene_node.h"
#include "plane.h"

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

			ngm::Vec3f forward;
			ngm::Vec3f up;
			ngm::Vec3f right;

			ngm::Vec3f rightPlaneNormal;
			ngm::Vec3f leftPlaneNormal;
			ngm::Vec3f topPlaneNormal;
			ngm::Vec3f bottomPlaneNormal;

			ngm::Plane farPlane;
			ngm::Plane nearPlane;
			ngm::Plane rightPlane;
			ngm::Plane leftPlane;
			ngm::Plane topPlane;
			ngm::Plane bottomPlane;


			CameraNode(float fov, float viewDistance, float htwRatio);

			void onUpdate(float time) override;

			~CameraNode();
		};

	}
}
