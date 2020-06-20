#pragma once

#include <memory>
#include <array>

#include "../def.h"
#include "resources.h"

namespace ngv {
	class VulkanDevice;
	class VulkanAllocator;
}

namespace ng {

	class AbstractFreeListAllocator;

	class VertexBufferPage;
	class IndexBufferPage;
	class UniformBufferPage;

	enum class ResourceResidencyFlag {
		eNoResidency,
		eDeviceResidency,
		eStagingResidency,
		eAllResidency = eDeviceResidency | eStagingResidency,
	};

	struct ResourceStrategy {
		uint64 stagingBufferPageSize;

		uint64 hostVertexBufferPageSize;
		uint64 deviceVertexBufferPageSize;

		uint64 hostIndexBufferPageSize;
		uint64 deviceIndexBufferPageSize;

		uint64 hostUniformBufferPageSize;
		uint64 deviceUniformBufferPageSize;
	};

	class ResourceManager {
	public:

		ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device, ResourceStrategy strategy);

		std::shared_ptr<StagingBuffer> getStagingBuffer(std::string filename);
		std::shared_ptr<VertexBuffer> getVertexBuffer(std::string filename);
		std::shared_ptr<IndexBuffer> getIndexBuffer(std::string filename);
		std::shared_ptr<UniformBuffer> getUniformBuffer(std::string filename);
		std::shared_ptr<Texture2D> getTexture2D(std::string filename);


		void giveDeviceAllocation(std::shared_ptr<VertexBuffer>& vertexBuffer);
		void giveDeviceAllocation(std::shared_ptr<IndexBuffer>& indexBuffer);
		void giveDeviceAllocation(std::shared_ptr<UniformBuffer>& uniformBuffer);
		void giveDeviceAllocation(std::shared_ptr<Texture2D>& texture2D);


		const ngv::VulkanDevice& vulkanDevice() const;


	private:

		bool shouldUseNewStagingMemory();

		bool shouldUseNewDeviceVertexMemory();
		bool shouldUseNewHostVertexMemory();

		bool shouldUseNewDeviceIndexMemory();
		bool shouldUseNewHostIndexMemory();

		bool shouldUseNewDeviceUniformMemory();
		bool shouldUseNewHostUniformMemory();

		bool shouldUseNewDeviceTexture2DMemory();
		bool shouldUseNewHostTexture2DMemory();

	private:

		ngv::VulkanAllocator& m_Allocator;
		ngv::VulkanDevice& m_Device;
		ResourceStrategy m_Strategy;

		vk::DeviceSize m_UsedDeviceMemory = 0;
		vk::DeviceSize m_UsedHostMemory = 0;

		//Buffers
		struct {
			std::map<std::string, std::shared_ptr<StagingBuffer>> stagingBuffersByID;
			std::map<std::string, std::shared_ptr<VertexBuffer>> vertexBuffersByID;
			std::map<std::string, std::shared_ptr<IndexBuffer>> indexBuffersByID;
			std::map<std::string, std::shared_ptr<UniformBuffer>> uniformBuffersByID;
		} m_Buffers;
		
		
		struct {
			std::map<std::string, std::shared_ptr<Texture2D>> texturesByID;
			std::map<std::string, ng::raw_ptr<Texture2D>> textureResidencyLists[2][2]; // [resourceResidency][requiredResourceResidency]
			//...
		} m_Texture2Ds;


		struct {
			std::list<StagingBufferPage> stagingBufferPages;

			std::list<VertexBufferPage> hostVertexBufferPages;
			std::list<VertexBufferPage> deviceVertexBufferPages;

			std::list<IndexBufferPage> hostIndexBufferPages;
			std::list<IndexBufferPage> deviceIndexBufferPages;

			std::list<UniformBufferPage> hostUniformBufferPages;
			std::list<UniformBufferPage> deviceUniformBufferPages;
		} m_BufferPages;


	};




	class StagingBufferPage {
	public:
		bool allocate(std::shared_ptr<StagingBuffer>& stagingBuffer);
	private:
		StagingBufferPage();
		StagingBufferPage(const VertexBufferPage&) = delete;
		StagingBufferPage& operator=(const StagingBufferPage&) = default;
	private:
		friend class ResourceManager;
		std::shared_ptr<ngv::VulkanBuffer> m_StagingBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_Allocator;
	};

	class VertexBufferPage {
	public:
		bool allocate(std::shared_ptr<VertexBuffer>& vertexBuffer);
	private:
		VertexBufferPage();
		VertexBufferPage(const VertexBufferPage&) = delete;
		VertexBufferPage& operator=(const VertexBufferPage&) = default;
	private:
		friend class ResourceManager;
		std::shared_ptr<ngv::VulkanVertexBuffer> m_VertexBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_Allocator;
	};

	class IndexBufferPage {
	public:
		bool allocate(std::shared_ptr<IndexBuffer>& indexBuffer);
	private:
		IndexBufferPage();
		IndexBufferPage(const IndexBufferPage&) = delete;
		IndexBufferPage& operator=(const IndexBufferPage&) = default;
	private:
		friend class ResourceManager;
		std::shared_ptr<ngv::VulkanIndexBuffer> m_IndexBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_Allocator;
	};

	class UniformBufferPage {
	public:
		bool allocate(std::shared_ptr<UniformBuffer>& uniformBuffer);
	private:
		UniformBufferPage();
		UniformBufferPage(const UniformBufferPage&) = delete;
		UniformBufferPage& operator=(const UniformBufferPage&) = default;
	private:
		friend class ResourceManager;
		std::shared_ptr<ngv::VulkanUniformBuffer> m_UniformBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_Allocator;
	};






}