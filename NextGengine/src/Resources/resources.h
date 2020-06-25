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

		bool hasAllocation();

		~StagingBuffer();

	private:
		friend class ResourceManager;
		friend class StagingBufferPage;

		StagingBuffer(ResourceManager& manager, std::string id);
		StagingBuffer(const StagingBuffer&) = delete;
		StagingBuffer& operator=(StagingBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<StagingBufferPage> m_pStagingPage;

	};














	class VertexBuffer {
	public:

		bool hasDeviceAllocation();
		bool hasStagingBuffer();

		void looseDeviceAllocation(bool alreadyLocked = false);
		void looseStagingBuffer(bool alreadyLocked = false);

		void giveDeviceAllocation(bool alreadyLocked = false);
		void giveStagingBuffer(bool alreadyLocked = false);

		~VertexBuffer();

	private:
		friend class ResourceManager;
		friend class VertexBufferPage;

		VertexBuffer(ResourceManager& manager, std::string id);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<VertexBufferPage> m_pVertexPage;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;

	};
	
















	class IndexBuffer {
	public:

		bool hasDeviceAllocation();
		bool hasStagingBuffer();

		void setDeviceAllocationNotRequired(bool alreadyLocked = false);
		void setStagingBufferNotRequired(bool alreadyLocked = false);

		void giveDeviceAllocation(bool alreadyLocked = false);
		void giveStagingBuffer(bool alreadyLocked = false);

		~IndexBuffer();

	private:
		friend class ResourceManager;
		friend class IndexBufferPage;

		IndexBuffer(ResourceManager& manager, std::string id);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<IndexBufferPage> m_pIndexPage;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;

	};














	class UniformBuffer {
	public:

		bool hasDeviceAllocation();
		bool hasStagingBuffer();

		void setDeviceAllocationNotRequired(bool alreadyLocked = false);
		void setStagingBufferNotRequired(bool alreadyLocked = false);

		void giveDeviceAllocation(bool alreadyLocked = false);
		void giveStagingBuffer(bool alreadyLocked = false);
		
		~UniformBuffer();

	private:
		friend class ResourceManager;
		friend class UniformBufferPage;

		UniformBuffer(ResourceManager& manager, std::string id);
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<UniformBufferPage> m_pUniformPage;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;

	};

















	class Texture2D {
	public:
		
		bool hasDeviceAllocation();
		bool hasStagingBuffer();

		void setDeviceAllocationNotRequired(bool alreadyLocked = false);
		void setStagingBufferNotRequired(bool alreadyLocked = false);

		void giveDeviceAllocation(bool alreadyLocked = false);
		void giveStagingBuffer(bool alreadyLocked = false);

	private:
		friend class ResourceManager;

		Texture2D(ResourceManager& manager, std::string id);
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(Texture2D&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;

		uint32 m_Width, m_Height;
		uint32 m_MipLevels;
		vk::Format m_Format;

		std::shared_ptr<ngv::VulkanTexture2D> m_pVulkanTexture;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;

	};

}