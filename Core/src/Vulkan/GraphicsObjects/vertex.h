#pragma once

#include "vec2f.h"
#include "vec3f.h"
#include "vec4f.h"
#include <unordered_map>

namespace ng {
	namespace vulkan {

		class Vertex5Component {
		public:
			ngm::Vec3f position;
			ngm::Vec2f uv;
			ngm::Vec3f color;
			ngm::Vec3f normal;
			ngm::Vec3f tangent;

			bool operator==(const Vertex5Component& other) const;
		};

		class Vertex4Component {
		public:
			ngm::Vec3f position;
			ngm::Vec3f normal;
			ngm::Vec2f uv;
			ngm::Vec3f color;

			bool operator==(const Vertex4Component& other) const;
		};

		class Vertex3Component {
		public:
			ngm::Vec3f position;
			ngm::Vec3f color;
			ngm::Vec2f texCoord;

			bool operator==(const Vertex3Component& other) const;
		};
	}
}

namespace std {
	template<> struct hash<ng::vulkan::Vertex5Component> {
		size_t operator()(ng::vulkan::Vertex5Component const& vertex) const {
			return hash<ngm::Vec3f>()(vertex.position) ^
				hash<ngm::Vec2f>()(vertex.uv) ^
				hash<ngm::Vec3f>()(vertex.color) ^
				hash<ngm::Vec3f>()(vertex.normal) ^
				hash<ngm::Vec3f>()(vertex.tangent);
		}
	};

	template<> struct hash<ng::vulkan::Vertex4Component> {
		size_t operator()(ng::vulkan::Vertex4Component const& vertex) const {
			return 10;
		}
	};

	template<> struct hash<ng::vulkan::Vertex3Component> {
		size_t operator()(ng::vulkan::Vertex3Component const& vertex) const {
			return
				((hash<ngm::Vec3f>()(vertex.position) ^
				(hash<ngm::Vec3f>()(vertex.color) << 1)) >> 1) ^
				(hash <ngm::Vec2f > ()(vertex.texCoord) << 1);
		}
	};

}

