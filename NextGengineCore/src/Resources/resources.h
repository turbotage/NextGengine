#pragma once

#include "pch.h"
#include "../ng_utility.h"


class ktxTexture;

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






	class Resource {
	public:

		Resource();

	protected:
		uint8 m_RequiredResidency;
		uint8 m_Residency;

	private:


	};













	class StagingBuffer : public Resource {
	public:

		~StagingBuffer();

	private:
		bool hasAllocation();
	private:
		friend class ResourceManager;
		friend class StagingBufferPage;
		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class UniformBuffer;
		friend class Texture2D;

		StagingBuffer(ResourceManager& manager, std::string id, uint64 size);
		StagingBuffer(const StagingBuffer&) = delete;
		StagingBuffer& operator=(StagingBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size = 0;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<StagingBufferPage> m_pStagingPage;

	};














	class VertexBuffer : public Resource {
	public:

		void giveDeviceAllocation(vk::CommandBuffer cb);
		void giveStagingBuffer();

		void setDeviceAllocationNotRequired();
		void setStagingBufferNotRequired();


		~VertexBuffer();

	private:
		bool hasDeviceAllocation();
		bool hasStagingBuffer();

	private:
		friend class ResourceManager;
		friend class VertexBufferPage;

		VertexBuffer(ResourceManager& manager, std::string id);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(VertexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size = 0;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<VertexBufferPage> m_pVertexPage = nullptr;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;
	};
	
















	class IndexBuffer : public Resource {
	public:

		void giveDeviceAllocation(vk::CommandBuffer cb);
		void giveStagingBuffer();

		void setDeviceAllocationNotRequired();
		void setStagingBufferNotRequired();


		~IndexBuffer();

	private:
		bool hasDeviceAllocation();
		bool hasStagingBuffer();
	private:
		friend class ResourceManager;
		friend class IndexBufferPage;

		IndexBuffer(ResourceManager& manager, std::string id);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(IndexBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size = 0;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<IndexBufferPage> m_pIndexPage = nullptr;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;

	};














	class UniformBuffer : public Resource {
	public:

		void giveDeviceAllocation(vk::CommandBuffer cb);
		void giveStagingBuffer();

		void setDeviceAllocationNotRequired();
		void setStagingBufferNotRequired();

		
		~UniformBuffer();

	private:
		bool hasDeviceAllocation();
		bool hasStagingBuffer();
	private:
		friend class ResourceManager;
		friend class UniformBufferPage;

		UniformBuffer(ResourceManager& manager, std::string id);
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(UniformBuffer&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;
		vk::DeviceSize m_Size = 0;

		std::unique_ptr<AbstractFreeListAllocation> m_pAllocation;
		ng::raw_ptr<UniformBufferPage> m_pUniformPage = nullptr;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;

	};
















	class Texture2D : public Resource {
	public:
		
		void giveDeviceAllocation(vk::CommandBuffer cb);
		void giveStagingBuffer();

		void setDeviceAllocationNotRequired();
		void setStagingBufferNotRequired();

		~Texture2D();

	private:
		bool hasDeviceAllocation();
		bool hasStagingBuffer();
	private:
		friend class ResourceManager;

		Texture2D(ResourceManager& manager, std::string id);
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(Texture2D&) = delete;
	private:
		ResourceManager& m_Manager;
		std::string m_ID;

		uint32 m_Width = 0;
		uint32 m_Height = 0;
		uint32 m_MipLevels = 0;
		vk::Format m_Format;

		std::shared_ptr<ngv::VulkanTexture2D> m_pVulkanTexture;

		std::shared_ptr<ng::StagingBuffer> m_pStagingBuffer;
		ktxTexture* m_KTXTexture = nullptr;

	};

}