#pragma once

#include <memory>
#include <array>
#include <mutex>
#include <unordered_map>

#include "../def.h"
#include "resources.h"

class ktxTexture;

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

		//std::shared_ptr<StagingBuffer> getStagingBuffer(std::string filename);

		std::shared_ptr<VertexBuffer> getVertexBuffer(std::string filename);
		std::shared_ptr<IndexBuffer> getIndexBuffer(std::string filename);
		std::shared_ptr<UniformBuffer> getUniformBuffer(std::string filename);
		std::shared_ptr<Texture2D> getTexture2D(std::string filename);

		void giveStagingBuffer(VertexBuffer& vertexBuffer);
		void giveStagingBuffer(IndexBuffer& indexBuffer);
		void giveStagingBuffer(UniformBuffer& uniformBuffer);
		void giveStagingBuffer(Texture2D& texture2D);

		void giveDeviceAllocation(VertexBuffer& vertexBuffer);
		void giveDeviceAllocation(IndexBuffer& indexBuffer);
		void giveDeviceAllocation(UniformBuffer& uniformBuffer);
		void giveDeviceAllocation(Texture2D& texture2D);

	private:

		std::shared_ptr<StagingBuffer> mGetStagingBuffer(std::string id, uint64 size);
		void uploadToStagingBuffer(StagingBuffer& buffer, void* data);


		void mGiveStagingBuffer(VertexBuffer& vertexBuffer);
		void mGiveStagingBuffer(IndexBuffer& indexBuffer);
		void mGiveStagingBuffer(UniformBuffer& uniformBuffer);
		void mGiveStagingBuffer(Texture2D& texture2D, ktxTexture* ktxTexture = nullptr);

		void mGiveDeviceAllocation(VertexBuffer& vertexBuffer);
		void mGiveDeviceAllocation(IndexBuffer& indexBuffer);
		void mGiveDeviceAllocation(UniformBuffer& uniformBuffer);
		void mGiveDeviceAllocation(Texture2D& texture2D);
		
	private:

		bool mShouldUseNewStagingMemory();

		bool mShouldUseNewDeviceVertexMemory();
		bool mShouldUseNewHostVertexMemory();

		bool mShouldUseNewDeviceIndexMemory();
		bool mShouldUseNewHostIndexMemory();

		bool mShouldUseNewDeviceUniformMemory();
		bool mShouldUseNewHostUniformMemory();

		bool mShouldUseNewDeviceTexture2DMemory();
		bool mShouldUseNewHostTexture2DMemory();

	private:


		std::mutex m_Mutex;

		ngv::VulkanAllocator& m_Allocator;
		ngv::VulkanDevice& m_Device;
		ResourceStrategy m_Strategy;

		vk::DeviceSize m_UsedDeviceMemory = 0;
		vk::DeviceSize m_UsedHostMemory = 0;

		struct {
			std::unordered_map<std::string, std::shared_ptr<StagingBuffer>> stagingResidencyMaps[2]; // Required, Not Required
		} m_Staging;

		//Buffers
		struct {
			std::unordered_map<std::string, std::shared_ptr<VertexBuffer>> vertexBuffersByID;
			std::unordered_map<std::string, std::shared_ptr<IndexBuffer>> indexBuffersByID;
			std::unordered_map<std::string, std::shared_ptr<UniformBuffer>> uniformBuffersByID;
		} m_Buffers;
		
		
		struct {
			std::unordered_map<std::string, std::shared_ptr<Texture2D>> texturesByID;
			std::unordered_map<std::string, ng::raw_ptr<Texture2D>> textureResidencyMaps[3][3]; // [resourceResidency][requiredResourceResidency]
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

		void free(StagingBuffer& stagingBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanBuffer> getBuffer();

	private:
		StagingBufferPage(const ResourceManager& manager);
		StagingBufferPage(const VertexBufferPage&) = delete;
		StagingBufferPage& operator=(const StagingBufferPage&) = default;
	private:
		friend class ResourceManager;

		//std::mutex m_Mutex;

		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanBuffer> m_pStagingBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};














	class VertexBufferPage {
	public:
		bool allocate(VertexBuffer& vertexBuffer);

		void free(VertexBuffer& vertexBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanVertexBuffer> getBuffer();

	private:
		VertexBufferPage(const ResourceManager& manager);
		VertexBufferPage(const VertexBufferPage&) = delete;
		VertexBufferPage& operator=(const VertexBufferPage&) = default;
	private:
		friend class ResourceManager;

		//std::mutex m_Mutex;

		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanVertexBuffer> m_pVertexBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};













	class IndexBufferPage {
	public:
		bool allocate(IndexBuffer& indexBuffer);

		void free(IndexBuffer& indexBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanIndexBuffer> getBuffer();

	private:
		IndexBufferPage(const ResourceManager& manager);
		IndexBufferPage(const IndexBufferPage&) = delete;
		IndexBufferPage& operator=(const IndexBufferPage&) = default;
	private:
		friend class ResourceManager;

		//std::mutex m_Mutex;

		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanIndexBuffer> m_pIndexBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};















	class UniformBufferPage {
	public:
		bool allocate(UniformBuffer& uniformBuffer);

		void free(UniformBuffer& uniformBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanUniformBuffer> getBuffer();

	private:
		UniformBufferPage(const ResourceManager& manager);
		UniformBufferPage(const UniformBufferPage&) = delete;
		UniformBufferPage& operator=(const UniformBufferPage&) = default;
	private:
		friend class ResourceManager;

		//std::mutex m_Mutex;

		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanUniformBuffer> m_pUniformBuffer;
		std::unique_ptr<AbstractFreeListAllocator> m_pAllocator;
	};






}