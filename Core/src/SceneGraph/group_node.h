#pragma once

#include "scene_graph.h"
#include <mutex>


namespace ng {
	namespace scenegraph {
		class GroupNode : SceneNode {
		private:
			std::mutex m_Mutex;
			

			void setCalculatedCombinedCenter();

		public:
			
		};
	}
}

