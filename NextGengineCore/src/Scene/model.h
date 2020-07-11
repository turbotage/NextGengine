#pragma once

#include <pch.h>

#include "scene.h"

namespace ng {

	class VertexBuffer;
	class IndexBuffer;
	class Texture2D;

	class ResourceManager;

	enum class AlphaModeFlagBits {
		eOpaque,
		eTranslucent
	};

	class Model : SceneNode {
	public:
		Model(std::string identifier);

	private:
		Model(const Model&) = delete;
		Model& operator=(Model&) = delete;

	private:
		// Transformation
		glm::mat4 m_Matrix; 
		// The mesh consists of some primitives
		std::vector<ModelPrimitive> m_Mesh;
		std::vector<ModelMaterial> m_Materials;
		

		std::shared_ptr<VertexBuffer> m_pVertexBuffer;
		std::shared_ptr<IndexBuffer> m_pIndexBuffer;
		std::vector<std::shared_ptr<Texture2D>> m_Textures;


		bool m_Visible = true;
	};






	class Model2D : SceneNode {
	public:

		Model2D(std::string identifier);
		

	private:
		Model2D(const Model2D&) = delete;
		Model2D& operator=(Model2D&) = delete;

		

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::shared_ptr<Texture2D> m_Texture;
	};

}