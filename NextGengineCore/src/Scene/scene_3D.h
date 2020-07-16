#pragma once

#include <pch.h>

#include "scene_utility.h"

namespace ng {

	class Model3D;
	class ResourceManager;
	class VertexBuffer;
	class IndexBuffer;
	class Texture2D;
	

	class SceneNode3D {
	public:
		
		SceneNode3D(SceneNode3D& parentNode, std::string nodeID);

	private:
		SceneNode3D(const SceneNode3D&) = delete;
		SceneNode3D& operator=(SceneNode3D&) = delete;

		friend class Scene;
		friend class SceneNode;
	private:
		SceneNode3D& m_Parent;
		std::vector<std::unique_ptr<SceneNode3D>> m_Children;

		std::string m_ID;

		// Transformation
		glm::mat4 m_Matrix;

		// The mesh consists of some primitives
		std::vector<ModelPrimitive> m_Primitives; // Mesh
		std::vector<std::shared_ptr<ModelMaterial>> m_Materials;


		std::shared_ptr<VertexBuffer> m_pVertexBuffer;
		std::shared_ptr<IndexBuffer> m_pIndexBuffer;
		std::vector<std::shared_ptr<Texture2D>> m_Textures;


		bool m_Visible = true;

	};

	class Scene3D {
	public:

		Scene3D(ResourceManager& manager, std::string& gltfFilename);

	private:

		// Load the materials that are in the scene
		void loadMaterials();

		void loadNodes();
		void loadNode(SceneNode3D& node, tinygltf::Node& gltfNode);

	private:

		tinygltf::Model m_GLTFModel;
		tinygltf::TinyGLTF m_GLTFContext;

		ResourceManager& m_Manager;
		
		std::map<std::string, std::shared_ptr<ModelMaterial>> m_Materials;


	};

	class SceneGraph3D {
	public:

		SceneGraph3D(Scene3D& scene, ResourceManager& manager);

	private:
		SceneGraph3D(SceneGraph3D&) = delete;
		SceneGraph3D& operator=(SceneGraph3D&) = delete;
	private:
		ResourceManager& m_Manager;
		Scene& m_Scene;

		std::unique_ptr<SceneNode> m_RootNode;

	};

	struct Model3DPrimitive {
		uint32 firstIndex;
		uint32 indexCount;
		int32 materialIndex;
	};

	struct Model3DMaterial {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);

		std::string baseColorTexturePath;
		uint32 normalTexturePath;

		uint8 alphaMode = AlphaModeFlagBits::eOpaque;
		float alphaCutOff;
		bool doubleSided = false;


	};

}