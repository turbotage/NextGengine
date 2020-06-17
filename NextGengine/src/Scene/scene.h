#pragma once

#include "../def.h"

#include <memory>

namespace ng {

	class Scene {

	};

	enum class SceneNodeType {
		eCamera,
		ePlayerController,
		eModel2D,
		eModel3D,
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

	class SceneGraph {
	public:

	private:

		std::multimap<std::string, std::unique_ptr<SceneNode>> m_SceneGraph;

	};

}