#pragma once

#include "scene_graph.h"

namespace ng {
	namespace scenegraph {
		class TransformNode : SceneNode {
		private:
			ng::math::Mat4 m_Transform;
			ng::math::Quaternion m_Rotation;
		public:

		};
	}
}