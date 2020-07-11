#pragma once

#include <pch.h>

#include "scene_utility.h"

namespace ng {

	class ResourceManager;

	enum class SceneNodeType {
		eCamera,
		ePlayerController,
		eModel2D,
		eModel,
	};

	class SceneNode {
	public:
		
		SceneNode(SceneNodeType nodeType, std::string nodeId);

	private:
		SceneNode(const SceneNode&) = delete;
		SceneNode& operator=(SceneNode&) = delete;

		ng::SceneNodeType m_NodeType;
		std::string m_Id;
	};

	class Scene {
	public:

		Scene(std::string& gltfFilename);

	private:

		// Load the materials that are in the scene
		void loadMaterials();

	private:

		tinygltf::Model m_Model;
		tinygltf::TinyGLTF m_Context;
		
		std::map<std::string, std::shared_ptr<ModelMaterial>> m_Materials;


	};

	class SceneGraph {
	public:

		SceneGraph(Scene& scene, ResourceManager& manager);

	private:
		SceneGraph(SceneGraph&) = delete;
		SceneGraph& operator=(SceneGraph&) = delete;
	private:
		ResourceManager& m_Manager;
		Scene& m_Scene;

		std::multimap<std::string, std::unique_ptr<SceneNode>> m_SceneGraph;

	};

}