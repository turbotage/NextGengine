#pragma once

#include "../def.h"
#include "../Vulkan/vulkandef.h"

namespace ngv {
	class VulkanBuffer;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class VulkanUniformBuffer;
	class VulkanTexture2D;
}

namespace ng {
	
	class ResourceManager;
	class AbstractFreeListAllocation;


	class StagingBuffer {
	public:

	private:
		friend class ResourceManager;

		StagingBuffer(ResourceManager& manager, std::string id);
		StagingBuffer(const StagingBuffer&) = delete;
		StagingBuffer& operator=(StagingBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanBuffer> m_pVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
	};

	class VertexBuffer {
	public:

		bool hasDeviceAllocation();
		bool hasStaging();

	private:
		friend class ResourceManager;

		VertexBuffer(ResourceManager& manager, std::string id);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanVertexBuffer> m_pDeviceVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pDeviceAllocation;

		std::shared_ptr<ngv::VulkanVertexBuffer> m_pHostVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pHostAllocation;
	};

	class IndexBuffer {
	public:

		bool hasDeviceAllocation();
		bool hasStaging();

	private:
		friend class ResourceManager;

		IndexBuffer(ResourceManager& manager, std::string id);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanIndexBuffer> m_pDeviceVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pDeviceAllocation;

		std::shared_ptr<ngv::VulkanIndexBuffer> m_pHostVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pHostAllocation;

	};

	class UniformBuffer {
	public:

		bool hasDeviceAllocation();
		bool hasStaging();

	private:
		friend class ResourceManager;

		UniformBuffer(ResourceManager& manager, std::string id);
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		std::shared_ptr<ngv::VulkanUniformBuffer> m_pDeviceVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pDeviceAllocation;

		std::shared_ptr<ngv::VulkanUniformBuffer> m_pHostVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pHostAllocation;

	};

	class Texture2D {
	public:
		
		bool hasDeviceAllocation();
		bool hasStaging();

	private:
		friend class ResourceManager;

		Texture2D(ResourceManager& manager, std::string id);
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(Texture2D&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_Id;

		uint32 m_Width, m_Height;
		uint32 m_MipLevels;
		vk::Format m_Format;


		std::shared_ptr<ngv::VulkanTexture2D> m_pVulkanTexture;

		std::shared_ptr<ngv::VulkanBuffer> m_pHostVulkanBuffer;
		std::unique_ptr<AbstractFreeListAllocation> m_pHostAllocation;

	};

}