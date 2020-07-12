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
