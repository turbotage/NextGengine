#pragma once

#include "../Props/collidable.h"
#include "../Props/renderable.h"
#include "../Entities/entity.h"

namespace ng {
	namespace entity {
		class Object : public Collidable, public Renderable, public Entity
		{
		private:

		public:

			Object();
			~Object();
		};

	}
}

