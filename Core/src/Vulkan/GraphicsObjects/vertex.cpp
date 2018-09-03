#include "vertex.h"

bool ng::vulkan::Vertex5Component::operator==(const Vertex5Component & other) const
{
	return
		(position == other.position)
		&&
		(uv == other.uv)
		&&
		(color == other.color)
		&&
		(normal == other.normal)
		&&
		(tangent == other.tangent);
}

bool ng::vulkan::Vertex4Component::operator==(const Vertex4Component & other) const
{
	return 
		(position == other.position)
		&&
		(normal == other.normal)
		&&
		(uv == other.uv)
		&&
		(color == other.color);
}


bool ng::vulkan::Vertex3Component::operator==(const Vertex3Component & other) const
{
	return
		(position == other.position)
		&&
		(color == other.color)
		&& 
		(texCoord == other.texCoord);
}
