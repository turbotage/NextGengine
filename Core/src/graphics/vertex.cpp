#include "vertex.h"

bool ng::graphics::VertexExtended::operator==(const VertexExtended & other) const
{
	return
		(position == other.position)
		&&
		(uv == other.uv)
		&&
		(normal == other.normal)
		&&
		(binormal == other.binormal)
		&&
		(tangent == other.tangent);
}

bool ng::graphics::Vertex::operator==(const Vertex & other) const
{
	return
		(position == other.position)
		&&
		(color == other.color)
		&&
		(texCoord == other.texCoord);
}
