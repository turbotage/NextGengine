#include "scene_3D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Resources/resource_manager.h"
#include "../Resources/resources.h"

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
		const tinygltf::Mesh mesh = m_GLTFModel.meshes[gltfNode.mesh];

		for (size_t i = 0; i < mesh.primitives.size(); ++i) {
			const tinygltf::Primitive& primitive = mesh.primitives[i];
			
		}

	}

}




