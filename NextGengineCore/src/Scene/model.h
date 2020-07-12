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

	private:
		
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