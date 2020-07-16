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

}

ng::Scene::Scene(std::string& gltfFilename)
{


}
