#include "scene_3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Resources/resource_manager.h"
#include "../Resources/resources.h"

typedef ng::Vertex3D_6 Vertex;

ng::SceneNode3D::SceneNode3D(SceneNode3D& parentNode, std::string nodeID)
	: m_Parent(parentNode), m_ID(nodeID)
{

}

ng::SceneGraph3D::SceneGraph3D(Scene3D& scene, ResourceManager& manager)
	: m_Scene(scene), m_Manager(manager)
{
}






ng::Scene3D::Scene3D(ResourceManager& manager, std::string& gltfFilename)
	: m_Manager(manager)
{
}

void ng::Scene3D::loadMaterials() {
	
	for (size_t i = 0; i < m_GLTFModel.materials.size(); ++i) {
		tinygltf::Material material = m_GLTFModel.materials[i];

		std::shared_ptr<ModelMaterial> pModelMaterial;

		if (material.values.find("baseColorFactor") != material.values.end()) {
			pModelMaterial->baseColorFactor = glm::make_vec4(material.values["baseColorFactor"].ColorFactor().data());
		}
		if (material.values.find("baseColorTexture") != material.values.end()) {
			int index = material.values["baseColorTexture"].TextureIndex();
			index = m_Model.textures[index].source;
			pModelMaterial->baseColorTexturePath = m_Model.images[index].uri;
		}
		if (material.additionalValues.find("normalTexture") != material.additionalValues.end()) {
			int index = material.additionalValues["normalTexture"].TextureIndex();
			index = m_Model.textures[index].source;
			pModelMaterial->normalTexturePath = m_Model.images[index].uri;
		}
		 
		pModelMaterial->alphaMode = material.alphaMode;
		pModelMaterial->alphaCutOff = (float)material.alphaCutoff;
		pModelMaterial->doubleSided = material.doubleSided;

		m_Materials.insert(material.name, std::move(pModelMaterial));

		

	}

}

void ng::Scene3D::loadNodes()
{
}

void ng::Scene3D::loadNode(SceneNode3D& node, tinygltf::Node& gltfNode)
{
	node.m_ID = gltfNode.name;

	// Get the local node matrix
	// It's either made up from translation, rotation, scale or a 4x4 matrix
	node.m_Matrix = glm::mat4(1.0f);
	if (gltfNode.translation.size() == 3) {
		node.m_Matrix = glm::translate(node.matrix, glm::vec3(glm::make_vec3(gltfNode.translation.data())));
	}
	if (gltfNode.rotation.size() == 4) {
		glm::quat q = glm::make_quat(gltfNode.rotation.data());
		node.m_Matrix *= glm::mat4(q);
	}
	if (gltfNode.scale.size() == 3) {
		node.m_Matrix = glm::scale(node.matrix, glm::vec3(glm::make_vec3(gltfNode.scale.data())));
	}
	if (gltfNode.matrix.size() == 16) {
		node.m_Matrix = glm::make_mat4x4(gltfNode.matrix.data());
	};

	// if the node has children we should load them
	for (size_t i = 0; i < gltfNode.children.size(); ++i) {
		tinygltf::Node childGLTFNode = m_GLTFModel.nodes[gltfNode.children[i]];
		std::unique_ptr<SceneNode3D> childNode = std::make_unique(node, childGLTFNode.name);
		node.m_Children.emplace_back(std::move(child));
		loadNode(*child, childGLTFNode);
	}

	if (gltfNode.mesh > -1) {

		std::function<std::vector<uint8>()> loadVertices = [this, &gltfNode]() {
			const tinygltf::Mesh mesh = m_GLTFModel.meshes[gltfNode.mesh];
			std::vector<uint8> vertexBytes;

			const float* positionBuffer = nullptr;
			const float* normalsBuffer = nullptr;
			const float* texCoordsBuffer = nullptr;
			const float* tangentsBuffer = nullptr;
			size_t totalVertexCount = 0;

			for (size_t i = 0; i < mesh.primitives.size(); ++i) {
				tinygltf::Primitive& primitive = mesh.primitives[i];
				auto attribute = primitive.attributes.find("POSITION");
				if (attribute != primitive.attributes.end()) {
					const tinygltf::Accessor& accessor = this->m_GLTFModel.accessors[attribute->second];
					totalVertexCount += accessor.count;
				}
			}

			vertexBytes.resize(totalVertexCount * sizeof(Vertex));

			size_t v = 0;
			for (size_t i = 0; i < mesh.primitives.size(); ++i) {
				tinygltf::Primitive& primitive = mesh.primitives[i];
				size_t vertexCount;

				auto attribute = primitive.attributes.find("POSITION");
				if (attribute != primitive.attributes.end()) {
					const tinygltf::Accessor& accessor = this->m_GLTFModel.accessors[attribute->second];
					const tinygltf::BufferView& view = this->m_GLTFModel.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(this->m_GLTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = accessor.count;
				}
				attribute = primitive.attributes.find("NORMAL");
				if (attribute != primitive.attributes.end()) {
					const tinygltf::Accessor& accessor = this->m_GLTFModel.accessors[attribute->second];
					const tinygltf::BufferView& view = this->m_GLTFModel.bufferViews[accessor.bufferView];
					normalsBuffer = reinterpret_cast<const float*>(&(this->m_GLTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				attribute = primitive.attributes.find("TEXCOORD_0");
				if (attribute != primitive.attributes.end()) {
					const tinygltf::Accessor& accessor = this->m_GLTFModel.accessors[attribute->second];
					const tinygltf::BufferView& view = this->m_GLTFModel.bufferViews[accessor.bufferView];
					texCoordsBuffer = reinterpret_cast<const float*>(&(this->m_GLTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				attribute = primitive.attributes.find("TANGENT");
				if (attribute != primitive.attributes.end()) {
					const tinygltf::Accessor& accessor = this->m_GLTFModel.accessors[attribute->second];
					const tinygltf::BufferView& view = this->m_GLTFModel.bufferViews[accessor.bufferView];
					tangentsBuffer = reinterpret_cast<const float*>(&(this->m_GLTFModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				for (; v < vertexCount; ++v) {
					Vertex vert{};
					vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
					vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
					vert.tangent = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4] : glm::vec4(0.0f));
					memcpy(&vertexBytes[v * sizeof(Vertex)], &vert, sizeof(T));
				}

			}


		};

		std::function<std::vector<uint8>()> loadIndices = [this, &gltfNode]() {
			const tinygltf::Mesh mesh = m_GLTFModel.meshes[gltfNode.mesh];
			std::vector<uint8> indexBytes;

			size_t indexCount = 0;


			for (size_t i = 0; i < mesh.primitives.size(); ++i) {
				tinygltf::Primitive& primitive = mesh.primitives[i];

				const tinygltf::Accessor& accessor = m_GLTFModel.meshes[primitive.indices];
				const tinygltf::BufferView& bufferView = m_GLTFModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = m_GLTFModel.buffers[bufferView.buffer];

				uint32 oldIndexCount = indexCount;
				indexCount += static_cast<uint32>(accessor.count);

				switch (accessor.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				{
					indexBytes.resize(indexCount * sizeof(uint32));
					memcpy(&indexBytes[oldIndexCount * sizeof(uint32)], &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32));
					break;
				}
				case TINYGLTF_COMPONENT_TYPE_SHORT:
				{
					indexBytes.resize(indexCount * sizeof(uint16));
					memcpy(&indexBytes[oldIndexCount * sizeof(uint16)], &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16));
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					indexBytes.resize(indexCount * sizeof(uint8));
					memcpy(&indexBytes[oldIndexCount * sizeof(uint8)], &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8));
					break;
				}
				default:
				{
#ifndef NDEBUG
					std::runtime_error("GLTF index componentType not allowed");
#endif
				}
				}
			}
			return indexBytes;
		};

		const tinygltf::Mesh mesh = m_GLTFModel.meshes[gltfNode.mesh];
		if (!mesh.name.empty()) {
			node.m_pVertexBuffer = m_Manager.getVertexBuffer(mesh.name, loadVertices);
			node.m_pIndexBuffer = m_Manager.getIndexBuffer(mesh.name, loadIndices);

			uint32 indexOffset = 0;
			uint32 indexCount;

			for (size_t i = 0; i < mesh.primitives.size(); ++i) {
				const tinygltf::Primitive& gltfPrimitive = mesh.primitives[i];
				const tinygltf::Accessor& gltfAccessor = m_GLTFModel.accessors[gltfPrimitive.indices];
				const tinygltf::Material& gltfMaterial = m_GLTFModel.accessors[gltfPrimitive.material];

				Model3DMaterial material;
				

				Model3DPrimitive primitive{};
				primitive.firstIndex = indexOffset;
				primitive.indexCount = static_cast<uint32>(accessor.count);
				primitive.materialIndex = 
			}

		}
	}

}




