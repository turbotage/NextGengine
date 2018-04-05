#pragma once

#include "scene_graph.h"
#include "camera_node.h"

namespace ng {
	namespace scenegraph {

#define NOTHING_IN_FRUSTRUM 0 //if neither model nor aabb is in frustrum
#define EVERYTHING_IN_FRUSTRUM 1 //whole aabb is in the frustrum therefore also model is in frustrum
#define MODEL_IN_FRUSTRUM 2 //if the model is in frustrum but not the entire aabb
#define AABB_IN_FRUSTRUM 4 //if the aabb is in the frustrum, this does not imply that model is frustrum

		class CullingWalker
		{
		private:

			Scene* m_Scene;

			int8 isInView(RenderableNode* node, CameraNode* camera);

		public:

			CullingWalker();
			CullingWalker(Scene* scene);

			~CullingWalker();

			void walk(CameraNode* camera);
		};
	}
}


