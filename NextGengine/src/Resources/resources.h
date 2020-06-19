#pragma once

#include "../def.h"

namespace ngv {
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class VulkanUniformBuffer;
	class VulkanTexture2D;
}

namespace ng {
	
	class ResourceManager;
	class AbstractFreeListAllocation;



	class VertexBuffer {
	public:

	private:
		friend class ResourceManager;

		VertexBuffer(ResourceManager& manager, std::string id);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanVertexBuffer> m_VulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_Allocation;

	};

	class IndexBuffer {
	public:

	private:
		friend class ResourceManager;

		IndexBuffer(ResourceManager& manager, std::string id);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanIndexBuffer> m_VulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_Allocation;

	};

	class UniformBuffer {
	public:


	private:
		friend class ResourceManager;

		UniformBuffer(ResourceManager& manager, std::string id);
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanUniformBuffer> m_VulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_Allocation;

	};

	class Texture2D {
	public:
		
	private:
		friend class ResourceManager;

		Texture2D(ResourceManager& manager, std::string id);
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(Texture2D&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanTexture2D> m_VulkanTexture;
	};

}