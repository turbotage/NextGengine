#pragma once

#include "../def.h"
#include "../BoundingVolumes/general_bv.h"

class Renderable
{
private:
	uint8 m_RenderProps;
	GeneralBV* m_BoundingVolume;
public:

	enum eRenderProps {
		RENDERING_ON = 1,
	};

	Renderable();
	Renderable(uint8 renderProps);
};
