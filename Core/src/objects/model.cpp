#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <algorithm>
#include "../graphics/vertex.h"

ng::entity::Model::Model()
{

}

ng::entity::Model::~Model()
{

}

void ng::entity::Model::load(const char * path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	std::vector<graphics::Vertex> vertices;
	std::vector<uint32> indices32;
	std::vector<uint16> indices16;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path)) {
		throw std::runtime_error(err);
	}

	std::unordered_map<graphics::Vertex, uint32> uniqueVertices = {};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			graphics::Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.uv = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices32.push_back(uniqueVertices[vertex]);
		}
	}

	if (*std::max_element(indices32.begin(), indices32.end()) < 65000) {
		std::copy(indices32.begin(), indices32.end(), indices16.begin());
	}
}
