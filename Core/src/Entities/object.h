#pragma once

#include "../Props/collidable.h"
#include "../Props/renderable.h"

namespace ng {
	namespace entity {
		class Object : public Collidable<Object>, public Renderable<Object>
		{
		private:

		public:

			Object();
			~Object();
		};

	}
}

