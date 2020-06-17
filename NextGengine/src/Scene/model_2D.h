#pragma once

#include "scene.h"

namespace ng {

	class VertexBuffer;
	class IndexBuffer;
	class Texture2D;

	class Model2D : SceneNode {
	public:
		Model2D(std::string identifier);

	private:

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::shared_ptr<Texture2D> m_Texture;
	};

}