#pragma once

#include "resources.h"

class ktxTexture;

namespace ngv {
	class VulkanDevice;
	class VulkanAllocator;

	class VulkanBuffer;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class VulkanUniformBuffer;
	class VulkanTexture2D;
}

namespace ng {
	class AbstractFreeListAllocator;

	class VertexBufferPage;
	class IndexBufferPage;
	class UniformBufferPage;

}

namespace ng {

	enum class ResourceResidencyFlagBits {
		eNoResidency,
		eDeviceResidency,
		eStagingResidency,
		eAllResidency = eDeviceResidency | eStagingResidency,
	};

	typedef uint32 ResourceResidencyFlags;

	/*
	This should ideally be setup such that multiples of the sizes fits
	as compactly as possible in vulkan allocator's memory pages
	*/
	struct ResourceStrategy {
		uint64 stagingBufferPageSize = 64 * 1024 * 1024LL;

		uint64 hostVertexBufferPageSize = 64 * 1024 * 1024LL;
		uint64 deviceVertexBufferPageSize = 64 * 1024 * 1024LL;

		uint64 hostIndexBufferPageSize = 64 * 1024 * 1024LL;
		uint64 deviceIndexBufferPageSize = 64 * 1024 * 1024LL;

		uint64 hostUniformBufferPageSize = 64 * 1024 * 1024LL;
		uint64 deviceUniformBufferPageSize = 64 * 1024 * 1024LL;
	};

	class ResourceManager {
	public:

		ResourceManager(ngv::VulkanAllocator& allocator, ngv::VulkanDevice& device, ResourceStrategy strategy);

		const ngv::VulkanDevice& vulkanDevice() const;

		//std::shared_ptr<StagingBuffer> getStagingBuffer(std::string filename);

		std::shared_ptr<VertexBuffer> getVertexBuffer(std::string& filename);
		std::shared_ptr<VertexBuffer> getVertexBuffer(std::string& m_ID);

		std::shared_ptr<IndexBuffer> getIndexBuffer(std::string& filename);
		std::shared_ptr<UniformBuffer> getUniformBuffer(std::string& filename);
		std::shared_ptr<Texture2D> getTexture2D(std::string& filename);
		
		/*
		void eraseVertexBuffer(std::string& filename);
		void eraseIndexBuffer(std::string& filename);
		void eraseUniformBuffer(std::string& filename);
		void eraseTexture2D(std::string& filename);
		*/

		void giveStagingBuffer(VertexBuffer& vertexBuffer);
		void giveStagingBuffer(IndexBuffer& indexBuffer);
		void giveStagingBuffer(UniformBuffer& uniformBuffer);
		void giveStagingBuffer(Texture2D& texture2D);

		void giveDeviceAllocation(VertexBuffer& vertexBuffer, vk::CommandBuffer cb);
		void giveDeviceAllocation(IndexBuffer& indexBuffer, vk::CommandBuffer cb);
		void giveDeviceAllocation(UniformBuffer& uniformBuffer, vk::CommandBuffer cb);
		void giveDeviceAllocation(Texture2D& texture2D, vk::CommandBuffer cb);

		void setDeviceAllocationNotRequired(VertexBuffer& vertexBuffer);
		void setDeviceAllocationNotRequired(IndexBuffer& indexBuffer);
		void setDeviceAllocationNotRequired(UniformBuffer& uniformBuffer);
		void setDeviceAllocationNotRequired(Texture2D& texture2D);

		void setStagingBufferNotRequired(VertexBuffer& vertexBuffer);
		void setStagingBufferNotRequired(IndexBuffer& indexBuffer);
		void setStagingBufferNotRequired(UniformBuffer& uniformBuffer);
		void setStagingBufferNotRequired(Texture2D& texture2D);

	private: // interface implementations

		std::shared_ptr<StagingBuffer> mGetStagingBuffer(std::string id, uint64 size);

		void mGiveStagingBuffer(VertexBuffer& vertexBuffer);
		void mGiveStagingBuffer(IndexBuffer& indexBuffer);
		void mGiveStagingBuffer(UniformBuffer& uniformBuffer);
		void mGiveStagingBuffer(Texture2D& texture2D);

		void mGiveDeviceAllocation(VertexBuffer& vertexBuffer, vk::CommandBuffer cb);
		void mGiveDeviceAllocation(IndexBuffer& indexBuffer, vk::CommandBuffer cb);
		void mGiveDeviceAllocation(UniformBuffer& uniformBuffer, vk::CommandBuffer cb);
		void mGiveDeviceAllocation(Texture2D& texture2D, vk::CommandBuffer cb);
		
		void mSetDeviceAllocationNotRequired(VertexBuffer& vertexBuffer);
		void mSetDeviceAllocationNotRequired(IndexBuffer& indexBuffer);
		void mSetDeviceAllocationNotRequired(UniformBuffer& uniformBuffer);
		void mSetDeviceAllocationNotRequired(Texture2D& texture2D);

		void mSetStagingBufferNotRequired(VertexBuffer& vertexBuffer);
		void mSetStagingBufferNotRequired(IndexBuffer& indexBuffer);
		void mSetStagingBufferNotRequired(UniformBuffer& uniformBuffer);
		void mSetStagingBufferNotRequired(Texture2D& texture2D);


	private: // helpers

		void mhUploadToStagingBuffer(StagingBuffer& buffer, uint8* data);

		void mhUploadToDevice(VertexBuffer& buffer, vk::CommandBuffer cb);
		void mhUploadToDevice(IndexBuffer& buffer, vk::CommandBuffer cb);
		void mhUploadToDevice(UniformBuffer& buffer, vk::CommandBuffer cb);
		void mhUploadToDevice(Texture2D& texture, vk::CommandBuffer cb);

		bool mhGiveStaging(Texture2D& texture2D);
		bool mhGiveStagingBySwapping(Texture2D& texture2D);
		bool mhGiveDeviceBySwapping(Texture2D& texture2D, vk::CommandBuffer cb);






		bool mhShouldUseNewStagingMemory();
		bool mhShouldUseNewDeviceVertexMemory();
		bool mhShouldUseNewHostVertexMemory();
		bool mhShouldUseNewDeviceIndexMemory();
		bool mhShouldUseNewHostIndexMemory();
		bool mhShouldUseNewDeviceUniformMemory();
		bool mhShouldUseNewHostUniformMemory();
		bool mhShouldUseNewDeviceTexture2DMemory();
		bool mhShouldUseNewHostTexture2DMemory();

	private:


		std::mutex m_Mutex;

		ngv::VulkanAllocator& m_Allocator;
		ngv::VulkanDevice& m_Device;
		ResourceStrategy m_Strategy;

		vk::DeviceSize m_UsedDeviceMemory = 0;
		vk::DeviceSize m_UsedHostMemory = 0;

		//Buffers
		struct {
			std::unordered_map<std::string, std::weak_ptr<VertexBuffer>> vertexBuffersByID;
			std::unordered_map<std::string, ng::raw_ptr<VertexBuffer>> vertexResidencyMaps[3][3]; // [resourceResidency][requiredResourceResidency]

			std::unordered_map<std::string, std::weak_ptr<IndexBuffer>> indexBuffersByID;
			std::unordered_map<std::string, ng::raw_ptr<IndexBuffer>> indexResidencyMaps[3][3]; // [resourceResidency][requiredResourceResidency]

			std::unordered_map<std::string, std::weak_ptr<UniformBuffer>> uniformBuffersByID;
			std::unordered_map<std::string, ng::raw_ptr<UniformBuffer>> uniformResidencyMaps[3][3]; // [resourceResidency][requiredResourceResidency]
		} m_Buffers;
		
		struct {
			std::unordered_map<std::string, std::weak_ptr<Texture2D>> texture2DsByID;
			std::unordered_map<std::string, ng::raw_ptr<Texture2D>> texture2DResidencyMaps[3][3]; // [resourceResidency][requiredResourceResidency]
			//...
		} m_Textures;


		struct {
			std::vector<std::unique_ptr<StagingBufferPage>> stagingBufferPages;

			std::vector<std::unique_ptr<VertexBufferPage>> vertexBufferPages;

			std::vector<std::unique_ptr<IndexBufferPage>> indexBufferPages;

			std::vector<std::unique_ptr<UniformBufferPage>> uniformBufferPages;
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
	private:
		friend class ResourceManager;

		//std::mutex m_Mutex;

		const ng::ResourceManager& m_Manager;

		std::shared_ptr<ngv::VulkanBuffer> m_pStagingBuffer;
		std::unique_ptr<ng::AbstractFreeListAllocator> m_pAllocator;
	};














	class VertexBufferPage {
	public:
		
		VertexBufferPage(const ResourceManager& manager);
		
		bool allocate(VertexBuffer& vertexBuffer);

		void free(VertexBuffer& vertexBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanVertexBuffer> getBuffer();

	private:
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
		
		IndexBufferPage(const ResourceManager& manager);
		
		bool allocate(IndexBuffer& indexBuffer);

		void free(IndexBuffer& indexBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanIndexBuffer> getBuffer();

	private:
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
		
		UniformBufferPage(const ResourceManager& manager);
		
		bool allocate(UniformBuffer& uniformBuffer);

		void free(UniformBuffer& uniformBuffer);

		const ResourceManager& getManager() const;

		ng::raw_ptr<ngv::VulkanUniformBuffer> getBuffer();

	private:
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