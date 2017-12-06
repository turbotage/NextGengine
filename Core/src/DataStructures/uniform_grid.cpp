#include "uniform_grid.h"

ng::dstructs::GridIndex ng::dstructs::UniformGrid::getIndex(ng::math::Vec3 position)
{
	GridIndex ret;
	ret.xIndex = (uint16)(position.x + 1);
	ret.yIndex = (uint16)(position.y + 1);
	ret.zIndex = (uint16)(position.z + 1);
	return ret;
}

ng::dstructs::UniformGrid::UniformGrid()
{

}

