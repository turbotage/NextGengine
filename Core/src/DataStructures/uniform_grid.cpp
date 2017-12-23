#include "uniform_grid.h"
#include <math.h>
#include "../Math/vec2.h"

ng::dstructs::GridIndex getDirection(float pitch, float yaw) {

	if ((pitch >= -NG_PI / 4.0f) || (pitch < NG_PI / 4.0f)) {
		return 1;
	}
	if ((pitch < -3.0f*NG_PI / 4.0f) || (pitch >= 3.0f*NG_PI / 4.0f)) {
		return 2;
	}
	if ((pitch >= NG_PI / 4.0f) && (pitch < 3.0f*NG_PI / 4.0f)) {
		return 3;
	}
	if ((pitch < -NG_PI / 4.0f) && (pitch >= -3.0f*NG_PI / 4.0f)) {
		return 4;
	}

}

ng::dstructs::GridIndex ng::dstructs::UniformGrid::getIndex(ng::math::Vec3 position)
{
	GridIndex ret;
	if (position.x >= 0) {
		ret.x = ((position.x / m_GridSize) + 1);
	}
	else {
		ret.x = ((position.x / m_GridSize) - 1);
	}
	if (position.y >= 0) {
		ret.y = ((position.y / m_GridSize) + 1);
	}
	else {
		ret.y = ((position.y / m_GridSize) - 1);
	}
	if (position.z >= 0) {
		ret.z = ((position.z / m_GridSize) + 1);
	}
	else {
		ret.z = ((position.z / m_GridSize) - 1);
	}
	return ret;
}

ng::math::Vec3 ng::dstructs::UniformGrid::getCenterPosition(const GridIndex * index)
{
	ng::math::Vec3 ret;
	if (index->x >= 0) {
		ret.x = (index->x * m_GridSize) - (m_GridSize / 2);
	}
	else {
		ret.x = (index->x * m_GridSize) + (m_GridSize / 2);
	}
	if (index->y >= 0) {
		ret.y = (index->x * m_GridSize) - (m_GridSize / 2);
	}
	else {
		ret.y = (index->x * m_GridSize) + (m_GridSize / 2);
	}
	if (index->z >= 0) {
		ret.z = (index->x * m_GridSize) - (m_GridSize / 2);
	}
	else {
		ret.z = (index->x * m_GridSize) + (m_GridSize / 2);
	}
	return ret;
}

std::vector<ng::dstructs::GridIndex*>* ng::dstructs::UniformGrid::gridIndicesCulling(Camera & cam)
{
	ng::math::Vec3 eulerAngles = cam.m_Rotation.eulerAngles(); //pitch, roll, yaw

	/*
	Vertical Plane					  Horizontal Plane
	View from along the Z-axis		  View from along the Y-axis
	-	  V2	-						-	  H2	-
	-	      -							  -		  -
	-	-								-	-
	V3	  -	   V1						H3	  -	   H1
	-	-								-	-
	-		  -							  -		  -
	-	  V4	-						-	  H4	-
	
	*/

#define Q1(s) ((s >= -NG_PI / 4.0f) || (s < NG_PI / 4.0f))
#define Q2(s) ((s >= NG_PI / 4.0f) && (s < 3.0f*NG_PI / 4.0f))
#define Q3(s) ((s < -3.0f*NG_PI / 4.0f) || (s >= 3.0f*NG_PI / 4.0f))
#define Q4(s) ((s < -NG_PI / 4.0f) && (s >= -3.0f*NG_PI / 4.0f))

	/*
	circle around
	*/

	//if the camera points somewhat along the positive or negative y axis
	if (Q2(eulerAngles.x) || Q4(eulerAngles.x)) {

	}
	//if the camera points somewhat along the positive or negative x axis
	else if (Q1(eulerAngles.z) || Q3(eulerAngles.z)) {

	}
	//if the camera points somewhat along the positive or negative z axis
	else if (Q2(eulerAngles.z) || Q4(eulerAngles.z)) {

	}

#undef V1
#undef V2
#undef V3
#undef V4

#undef H1
#undef H2
#undef H3
#undef H4
}

std::vector<Renderable*>* ng::dstructs::UniformGrid::frustrumCull(Camera & cam)
{

	__m128 rightPlane =
		_mm_set_ps(cam.rightPlane.a, cam.rightPlane.b, cam.rightPlane.c, cam.rightPlane.d);
	__m128 leftPlane =
		_mm_set_ps(cam.leftPlane.a, cam.leftPlane.b, cam.leftPlane.c, cam.leftPlane.d);
	__m128 topPlane =
		_mm_set_ps(cam.topPlane.a, cam.topPlane.b, cam.topPlane.c, cam.topPlane.d);
	__m128 bottomPlane =
		_mm_set_ps(cam.bottomPlane.a, cam.bottomPlane.b, cam.bottomPlane.c, cam.bottomPlane.d);
	__m128 nearPlane =
		_mm_set_ps(cam.nearPlane.a, cam.nearPlane.b, cam.nearPlane.c, cam.nearPlane.d);
	__m128 farPlane =
		_mm_set_ps(cam.farPlane.a, cam.farPlane.b, cam.farPlane.c, cam.farPlane.d);



}

ng::dstructs::UniformGrid::UniformGrid()
{
	
}

ng::dstructs::UniformGrid::UniformGrid(ng::math::Vec3s worldSizeMultiplier, float gridSize, uint16 expectedNumOfObjects)
	: m_GridSize(gridSize), m_WorldSizeMultipler(worldSizeMultiplier)
{
	float t = gridSize * 0.5;
	m_GridIndexRadius = sqrt(3 * t * t);
}
