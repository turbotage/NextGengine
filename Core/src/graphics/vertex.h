#pragma once

#include "../math/vec2.h"
#include "../math/vec3.h"
#include <unordered_map>

namespace ng {
	namespace graphics {
		class VertexExtended
		{
		public:
			math::Vec3 position;
			math::Vec2 uv;
			math::Vec3 normal;
			math::Vec3 binormal;
			math::Vec3 tangent;

			bool operator==(const VertexExtended& other) const;
		};

		class Vertex {
		public:
			math::Vec3 position;
			math::Vec3 color;
			math::Vec2 texCoord;

			bool operator==(const Vertex& other) const;
		};
	}
}

namespace std {
	template<> struct hash<ng::graphics::VertexExtended> {
		size_t operator()(ng::graphics::VertexExtended const& vertex) const {
			return hash<ng::math::Vec3>()(vertex.position) ^
				hash<ng::math::Vec2>()(vertex.uv) ^
				hash<ng::math::Vec3>()(vertex.normal) ^
				hash<ng::math::Vec3>()(vertex.binormal) ^
				hash<ng::math::Vec3>()(vertex.tangent);
		}
	};

	template<> struct hash<ng::graphics::Vertex> {
		size_t operator()(ng::graphics::Vertex const& vertex) const {
			return
				((hash<ng::math::Vec3>()(vertex.position) ^
				(hash<ng::math::Vec3>()(vertex.color) << 1)) >> 1) ^
				(hash < ng::math::Vec2 > ()(vertex.texCoord) << 1);
		}
	};

}

