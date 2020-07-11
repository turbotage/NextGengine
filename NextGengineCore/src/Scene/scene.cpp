#include "scene.h"


ng::SceneNode::SceneNode(SceneNodeType nodeType, std::string nodeId)
	: m_NodeType(nodeType), m_Id(nodeId)
{

}

ng::SceneGraph::SceneGraph(Scene& scene, ResourceManager& manager)
	: m_Scene(scene), m_Manager(manager)
{
}

void ng::Scene::loadMaterials() {

	for (size_t i = 0; i < m_Model.materials.size(); ++i) {
		tinygltf::Material material = m_Model.materials[i];

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

ng::Scene::Scene(std::string& gltfFilename)
{
	std::string error, warning;

	bool fileLoaded = gltfContext.LoadASCIIFromFile(&m_Model, &error, &warning, gltfFilename);

#ifndef NDEBUG
	if (!fileLoaded) {
		std::runtime_error("GLTF file could not be loaded");
	}
#endif

	loadMaterials();

}
