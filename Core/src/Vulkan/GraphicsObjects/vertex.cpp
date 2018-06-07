#include "vertex.h"

bool ng::vulkan::Vertex::operator==(const Vertex & other) const
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

bool ng::vulkan::VertexSimple::operator==(const VertexSimple & other) const
{
	return
		(position == other.position)
		&&
		(color == other.color)
		&& 
		(texCoord == other.texCoord);
}
