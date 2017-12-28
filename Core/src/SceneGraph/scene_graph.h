#pragma once

#include "../Math/mat4.h"
#include "../Math/quaternion.h"
#include "../Entities/mesh.h"
#include "../BoundingVolumes/general_bv.h"
#include "../BoundingVolumes/bounding_sphere.h"

namespace ng {
	namespace scenegraph {

		class SceneNode {
		private:

		protected:
			//tree-related
			SceneNode * m_Parent;
			std::vector<SceneNode*> m_Children;
			std::string m_Name;

			//positional
			ng::math::Vec3 m_Position;

			//BV
			ng::bvolumes::BoundingSphere m_BoundingSphere;

		public:

			SceneNode();

			ng::math::Vec3 getCenterPosition();

			float getBoundingSphereRadius();
			void setBoundingSphereRadius(float radius);

		};

		class SceneGraph
		{
		private:

		public:

			


		};
	}
}



