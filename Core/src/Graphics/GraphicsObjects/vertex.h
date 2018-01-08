#pragma once

#include "../../math/Vec2f.h"
#include "../../math/Vec3f.h"
#include <unordered_map>

namespace ng {
	namespace graphics {
		class VertexExtended
		{
		public:
			math::Vec3f position;
			math::Vec2f uv;
			math::Vec3f normal;
			math::Vec3f binormal;
			math::Vec3f tangent;

			bool operator==(const VertexExtended& other) const;
		};

		class Vertex {
		public:
			math::Vec3f position;
			math::Vec3f color;
			math::Vec2f texCoord;

			bool operator==(const Vertex& other) const;
		};
	}
}

namespace std {
	template<> struct hash<ng::graphics::VertexExtended> {
		size_t operator()(ng::graphics::VertexExtended const& vertex) const {
			return hash<ng::math::Vec3f>()(vertex.position) ^
				hash<ng::math::Vec2f>()(vertex.uv) ^
				hash<ng::math::Vec3f>()(vertex.normal) ^
				hash<ng::math::Vec3f>()(vertex.binormal) ^
				hash<ng::math::Vec3f>()(vertex.tangent);
		}
	};

	template<> struct hash<ng::graphics::Vertex> {
		size_t operator()(ng::graphics::Vertex const& vertex) const {
			return
				((hash<ng::math::Vec3f>()(vertex.position) ^
				(hash<ng::math::Vec3f>()(vertex.color) << 1)) >> 1) ^
				(hash < ng::math::Vec2f > ()(vertex.texCoord) << 1);
		}
	};

}

