#pragma once

#include <memory>
#include <array>
#include <mutex>

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

	enum class ResourceResidencyFlagBits {
		eNoResidency,
		eDeviceResidency,
		eStagingResidency,
		eAllResidency = eDeviceResidency | eStagingResidency,
	};

	typedef uint32 ResourceResidencyFlags;

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

		const ngv::VulkanDevice& vulkanDevice() const;

		std::shared_ptr<StagingBuffer> getStagingBuffer(std::string filename);
		std::shared_ptr<VertexBuffer> getVertexBuffer(std::string filename);
		std::shared_ptr<IndexBuffer> getIndexBuffer(std::string filename);
		std::shared_ptr<UniformBuffer> getUniformBuffer(std::string filename);
		std::shared_ptr<Texture2D> getTexture2D(std::string filename);

	private:

		void giveStagingBuffer(VertexBuffer& vertexBuffer);
		void giveStagingBuffer(IndexBuffer& indexBuffer);
		void giveStagingBuffer(UniformBuffer& uniformBuffer);
		void giveStagingBuffer(Texture2D& texture2D);

		void giveDeviceAllocation(VertexBuffer& vertexBuffer);
		void giveDeviceAllocation(IndexBuffer& indexBuffer);
		void giveDeviceAllocation(UniformBuffer& uniformBuffer);
		void giveDeviceAllocation(Texture2D& texture2D);

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
		friend class StagingBuffer;
		friend class VertexBuffer;
		friend class IndexBuffer;
		friend class UniformBuffer;
		friend class Texture2D;


		std::mutex m_Mutex;

		ngv::VulkanAllocator& m_Allocator;
		ngv::VulkanDevice& m_Device;
		ResourceStrategy m_Strategy;

		vk::DeviceSize m_UsedDeviceMemory = 0;
		vk::DeviceSize m_UsedHostMemory = 0;

		struct {
			std::map<std::string, std::shared_ptr<StagingBuffer>> stagingResidencyMaps[2]; // Required, Not Required
		} m_Staging;

		//Buffers
		struct {
			std::map<std::string, std::shared_ptr<VertexBuffer>> vertexBuffersByID;
			std::map<std::string, std::shared_ptr<IndexBuffer>> indexBuffersByID;
			std::map<std::string, std::shared_ptr<UniformBuffer>> uniformBuffersByID;
		} m_Buffers;
		
		
		struct {
			std::map<std::string, std::shared_ptr<Texture2D>> texturesByID;
			std::map<std::string, ng::raw_ptr<Texture2D>> textureResidencyMaps[3][3]; // [resourceResidency][requiredResourceResidency]
			//...
		} m_Texture2Ds;


		struct {
			std::list<StagingBufferPage> stagingBufferPages;

			std::list<VertexBufferPage> vertexBufferPages;

			std::list<IndexBufferPage> indexBufferPages;

			std::list<UniformBufferPage> uniformBufferPages;
		} m_BufferPages;


	};




	class StagingBufferPage {
	public:
		bool allocate(StagingBuffer& stagingBuffer);

		const ResourceManager& getManager() const;
	private:
		StagingBufferPage(const ResourceManager& manager);
		StagingBufferPage(const VertexBufferPage&) = delete;
		StagingBufferPage& operator=(const StagingBufferPage&) = default;
	private:
		friend class ResourceManager;
		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanBuffer> m_pStagingBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};

	class VertexBufferPage {
	public:
		bool allocate(VertexBuffer& vertexBuffer);

		const ResourceManager& getManager() const;
	private:
		VertexBufferPage(const ResourceManager& manager);
		VertexBufferPage(const VertexBufferPage&) = delete;
		VertexBufferPage& operator=(const VertexBufferPage&) = default;
	private:
		friend class ResourceManager;
		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanVertexBuffer> m_pVertexBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};

	class IndexBufferPage {
	public:
		bool allocate(IndexBuffer& indexBuffer);

		const ResourceManager& getManager() const;
	private:
		IndexBufferPage(const ResourceManager& manager);
		IndexBufferPage(const IndexBufferPage&) = delete;
		IndexBufferPage& operator=(const IndexBufferPage&) = default;
	private:
		friend class ResourceManager;
		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanIndexBuffer> m_pIndexBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};

	class UniformBufferPage {
	public:
		bool allocate(UniformBuffer& uniformBuffer);

		const ResourceManager& getManager() const;
	private:
		UniformBufferPage(const ResourceManager& manager);
		UniformBufferPage(const UniformBufferPage&) = delete;
		UniformBufferPage& operator=(const UniformBufferPage&) = default;
	private:
		friend class ResourceManager;
		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanUniformBuffer> m_pUniformBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};






}