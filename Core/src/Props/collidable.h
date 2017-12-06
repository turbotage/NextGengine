#pragma once

#include "../BoundingVolumes/general_bv.h"
#include "../def.h"

class Collidable
{
private:

	uint8 m_CollisionProps;
	GeneralBV* m_BoundingVolume;
public:

	enum eCollisionProps {
		COLLISION_ON = 1,
	};

	Collidable();
	Collidable(uint8 collisionProps);
};



