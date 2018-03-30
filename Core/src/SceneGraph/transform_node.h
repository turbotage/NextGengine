#pragma once

#include "scene_graph.h"

#define SCENE_GRAPH_NODE_TYPE_ID_TRANSFORM 1;

namespace ng {
	namespace scenegraph {

		class TransformNode : SceneNode {
		private:
			
		public:

			static TransformNode* castSceneNode(SceneNode* node) {
				return static_cast<TransformNode*>(node);
			}

		};
	}
}