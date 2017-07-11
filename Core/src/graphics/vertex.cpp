#include "vertex.h"

bool ng::graphics::Vertex::operator==(const Vertex & other) const
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
