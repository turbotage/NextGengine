#pragma once

#include "scene_graph.h"
#include "camera_node.h"

namespace ng {
	namespace scenegraph {
		class CullingWalker
		{
		private:
			Scene* m_Scene;

			int8 isInView(SceneNode* node);

		public:

			CullingWalker();
			CullingWalker(Scene* scene);

			~CullingWalker();

			void walk(CameraNode* camera);
		};
	}
}


