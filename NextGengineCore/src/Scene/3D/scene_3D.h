#pragma once

#include <pch.h>

#include <glm/gtx/hash.hpp>

#include "../scene_utility.h"

namespace ng {

	class ResourceManager;
	class SceneNode3D;
	class Material3D;
	class Primitive3D;



	class Scene3D {
	public:

		Scene3D(ResourceManager& manager);

		void loadScene(std::string filename);
	
	private:


		// Load the materials that are in the scene
		void loadMaterials();

		void loadNodes();
		void loadNode(SceneNode3D& node, tinygltf::Node& gltfNode, std::unordered_set<std::string>& nodeTracking);

	private:

		tinygltf::TinyGLTF m_GLTFContext;
		
		tinygltf::Model m_GLTFModel;

		ResourceManager& m_Manager;
		
		std::map<std::string, std::shared_ptr<Material3D>> m_Materials;

		// The root node in the scenegraph
		std::unique_ptr<SceneNode3D> m_pRootNode;
		

	};








	class SceneNode3D {
	public:

		SceneNode3D(SceneNode3D& parentNode, std::string nodeID);

		~SceneNode3D();

	private:
		SceneNode3D(std::string nodeID); // Used for root nodes
		SceneNode3D(const SceneNode3D&) = delete;
		SceneNode3D& operator=(SceneNode3D&) = delete;

		friend class Scene3D;
		friend class SceneNode3D;
	private:
		ng::raw_ptr<SceneNode3D> m_pParent;
		std::vector<std::unique_ptr<SceneNode3D>> m_Children;

		std::string m_ID;

		// Transformation
		glm::mat4 m_Matrix;

		// The mesh consists of some primitives
		std::vector<Primitive3D> m_Primitives; // Mesh

		std::shared_ptr<VertexBuffer> m_pVertexBuffer;
		std::shared_ptr<IndexBuffer> m_pIndexBuffer;


		bool m_Visible = true;

	};





	struct Material3D {
		std::shared_ptr<Texture2D> baseColorTexture;
		std::shared_ptr<Texture2D> normalTexture;

		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		uint8 alphaMode = AlphaModeFlagBits::eOpaque;
		float alphaCutOff = 0.0f;
		bool doubleSided = false;

	};

	/* Describes part of a mesh, the firstIndex and indexCount is relative to the meshes indexBuffer */
	struct Primitive3D {
		uint32 firstIndex;
		uint32 indexCount;
		std::shared_ptr<Material3D> material;
	};

























	/* DEPRECATED
	class SceneGraph3D {
	public:

		SceneGraph3D(Scene3D& scene, ResourceManager& manager);

	private:
		SceneGraph3D(SceneGraph3D&) = delete;
		SceneGraph3D& operator=(SceneGraph3D&) = delete;
	private:
		ResourceManager& m_Manager;
		Scene3D& m_Scene;

	};
	*/



}