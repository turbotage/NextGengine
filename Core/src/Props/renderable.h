#pragma once

#include "../def.h"
#include "../Entities/entity.h"
#include <mutex>

class Renderable
{
private:
	uint8 m_RenderProps;
	float m_Radius;
	ng::entity::Entity* m_Entity;
	std::mutex* m_Mutex;
public:

	enum eRenderProps {
		RENDERING_ON = 1,
	};

	Renderable();
	Renderable(uint8 renderProps);
};
