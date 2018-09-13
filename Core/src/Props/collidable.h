#pragma once

#include "../def.h"
#include "vec3f.h"
#include "../Entities/entity.h"
#include <mutex>

class Collidable
{
private:
	uint8 CollisionProps;
	ngm::Vec3f Dimensions;

	ng::entity::Entity* Entity;
	std::mutex* Mutex;
public:

	enum eCollisionProps {
		COLLISION_ON = 1,
	};

	Collidable();
	Collidable(uint8 collisionProps);
};



