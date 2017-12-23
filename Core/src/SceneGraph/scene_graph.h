#pragma once

#include "../Math/mat4.h"
#include "../Math/quaternion.h"
#include "../Entities/mesh.h"
#include "../BoundingVolumes/general_bv.h"

namespace ng {
	namespace scenegraph {

		class SceneNode {
		private:

		protected:
			//tree-related
			SceneNode * m_Parent;
			std::string name;

			//positional
			ng::math::Vec3 m_CenterPosition;
			ng::math::Vec3 m_AaBbDimensions; //width, height, depth
			float m_Radius;
		public:

			SceneNode();

			ng::math::Vec3 

		};

		class SceneGraph
		{
		private:

		public:

			


		};
	}
}



