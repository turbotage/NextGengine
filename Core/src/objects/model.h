#pragma once

#include "mesh.h"

namespace ng {
	namespace entity {
		class Model : Entity
		{
		private:
			VkPipeline pipeline;
			Mesh* m_Mesh;
		public:

			void loadPreformated(const char* path);
			
			void loadUnformated(const char* path);

		};
	}
}

