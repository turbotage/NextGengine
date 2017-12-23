#pragma once

#include "scene_graph.h"

namespace ng {
	namespace scenegraph {
		class GroupNode : SceneNode {
		private:
			std::vector<SceneNode*> m_Children;

			void calcCombinedCenter();
		public:
			void addChild(SceneNode* child);
		};
	}
}

