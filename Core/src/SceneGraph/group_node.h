#pragma once

#include "scene_graph.h"

namespace ng {
	namespace scenegraph {
		class GroupNode : SceneNode {
		private:

			void setCalculatedCombinedCenter();

		public:
			void addChild(SceneNode* child);
		};
	}
}

