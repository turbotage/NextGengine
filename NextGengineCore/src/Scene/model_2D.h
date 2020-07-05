#pragma once

#include "scene.h"

namespace ng {

	class VertexBuffer;
	class IndexBuffer;
	class Texture2D;

	class Model2D : SceneNode {
	public:

		

	private:
		Model2D(std::string identifier);
		Model2D(const Model2D&) = delete;
		Model2D& operator=(Model2D&) = delete;

		

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::shared_ptr<Texture2D> m_Texture;
	};

}