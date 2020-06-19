#pragma once

#include "../def.h"
#include "vulkandef.h"

#include <list>

#include <utility>
#include <vector>

namespace ngv {


	//Forward declarations
	class VulkanAllocator;
	class VulkanMemoryPage;
	class VulkanMemoryAllocation;	
	class VulkanDevice;


	// Base Buffer Class
	class VulkanBuffer : public ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanBuffer> make(VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer = false);

		~VulkanBuffer() = default; // allocations should be RAII deallocated
		
		vk::Buffer buffer() const;
		const vk::BufferCreateInfo getBufferCreateInfo() const;
		const vk::MemoryPropertyFlags getMemoryPropertyFlags() const;

		void* map();
		void unmap();


		void updateLocal(const void* value, vk::DeviceSize size) const;

		template<class T, class Allocator>
		void updateLocal(const std::vector<T, Allocator>& values) const;

		template<class T>
		void updateLocal(const T& value) const;



		void upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const void* value, vk::DeviceSize size);
		
		template<typename T>
		void upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values);
		
		template<typename T>
		void upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value);


		void barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
			vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, 
			uint32 srcQueueFamilyIndex, uint32 dstQueueFamilyIndex) const;

		void flush();

		void invalidate();

		bool hasAllocation();
		bool hasSameAllocation(const VulkanBuffer& buffer);

	protected:

		void create(VulkanDevice& device, 
			const vk::BufferCreateInfo& info, bool hostBuffer = false);

	protected:
		VulkanBuffer() = default;
		VulkanBuffer(VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer = false);
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

	private:
		friend class VulkanAllocator;
		vk::BufferCreateInfo m_BufferCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;

		vk::UniqueBuffer m_Buffer;
		vk::MemoryRequirements m_MemoryRequirements;
		uint32 m_MemoryTypeIndex = 0;

		bool m_Created = false;

		std::shared_ptr<VulkanMemoryAllocation> m_pAllocation;
		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;

	};

	template<class T, class Allocator>
	inline void VulkanBuffer::updateLocal(const std::vector<T, Allocator>& values) const
	{
		updateLocal( (void*)values.data(), (vk::DeviceSize)values.size() * sizeof(T) );
	}

	template<class T>
	inline void VulkanBuffer::updateLocal(const T& value) const
	{
		updateLocal((void*)&value, vk::DeviceSize(sizeof(T)));
	}

	template<typename T>
	inline void VulkanBuffer::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values)
	{
		upload(cb, stagingBuffer, (void*)values.data(), (uint64)values.size());
	}

	template<typename T>
	inline void VulkanBuffer::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value)
	{
		upload(cb, stagingBuffer, (void*)&value, sizeof(value));
	}


	



	// NOT IMPLEMENTED, DON'T USE (Vulkan Spare Buffer)
	/*
	class VulkanSparseBuffer : public ng::MakeConstructed<VulkanSparseBuffer> {
	public:

		static std::unique_ptr<VulkanSparseBuffer> make(VulkanDevice& device, const vk::BufferCreateInfo& info);

	protected:
		void create(VulkanDevice& device, const vk::BufferCreateInfo& info);
	protected:
		VulkanSparseBuffer() = default;
		VulkanSparseBuffer(VulkanDevice& device, const vk::BufferCreateInfo& info);
		VulkanSparseBuffer(const VulkanSparseBuffer&) = delete;
		VulkanSparseBuffer& operator=(const VulkanSparseBuffer&) = delete;
	private:
		friend class VulkanAllocator;
		vk::BufferCreateInfo m_BufferCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;
		vk::BindSparseInfo m_BindInfo;

		vk::UniqueBuffer m_Buffer;
		vk::MemoryRequirements m_MemoryRequirements;
		uint32 m_MemoryTypeIndex = 0;

		struct SparseBufferPage {
			vk::SparseBufferMemoryBindInfo bufferMemoryBind;
			VkDeviceSize size;
			uint32 index;

			std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;
			std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;
		};
		std::vector<SparseBufferPage> m_SparseBufferPages;


	};

	*/






	// Base Image Class
	class VulkanImage : public ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanImage> make(VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage = false);

		~VulkanImage() = default;

		void createImageView(vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask);

		// GETTERS
		vk::Image image() const;
		vk::ImageView imageView() const;
		//vk::DeviceMemory mem() const;
		vk::Format format() const;
		vk::Extent3D extent() const;
		const vk::ImageCreateInfo imageCreateInfo() const;
		const vk::MemoryPropertyFlags memoryPropertyFlags() const;

		void clear(vk::CommandBuffer cb, const std::array<float, 4> color = { 1,1,1,1 });


		void copy(vk::CommandBuffer cb, ngv::VulkanImage& srcImage);

		void copy(vk::CommandBuffer cb, vk::Buffer buffer, uint32 mipLevel,
			uint32 arrayLayer, uint32 width, uint32 height, uint32 depth, uint32 offset);


		void upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const void* value, vk::DeviceSize size);

		template<typename T>
		void upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values);

		template<typename T>
		void upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value);

		void setLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

		/// Set what the image thinks is its current layout (ie. the old layout in an image barrier).
		void setCurrentLayout(vk::ImageLayout oldLayout);

		bool hasAllocation();
		bool hasSameAllocation(const VulkanImage& image);

	protected:

		void create(VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage = false);

	protected:
		VulkanImage() = default;
		VulkanImage(VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage = false);
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

	private:
		friend class VulkanAllocator;

		vk::ImageCreateInfo m_ImageCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;
		vk::ImageLayout m_CurrentLayout = vk::ImageLayout::eUndefined;

		vk::UniqueImage m_Image;
		vk::UniqueImageView m_ImageView;
		vk::MemoryRequirements m_MemoryRequirements;
		uint32 m_MemoryTypeIndex = 0;

		bool m_Created = false;

		std::shared_ptr<VulkanMemoryAllocation> m_pAllocation;
		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;
	};

	template<typename T>
	inline void VulkanImage::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values)
	{
		upload(cb, stagingBuffer, (void*)values.data(), (uint32)values.size());
	}

	template<typename T>
	inline void VulkanImage::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value)
	{
		upload(cb, stagingBuffer, (void*)&value, sizeof(T));
	}









	class VulkanVertexBuffer : public VulkanBuffer, public ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanVertexBuffer> make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VulkanVertexBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) = delete;
	};











	class VulkanIndexBuffer : public VulkanBuffer, public ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanIndexBuffer> make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VulkanIndexBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VulkanIndexBuffer(const VulkanIndexBuffer&) = delete;
		VulkanIndexBuffer& operator=(const VulkanIndexBuffer&) = delete;
	}; 









	class VulkanUniformBuffer : public VulkanBuffer, public ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanUniformBuffer> make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VulkanUniformBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VulkanUniformBuffer(const VulkanUniformBuffer&) = delete;
		VulkanUniformBuffer& operator=(const VulkanUniformBuffer&) = delete;
	};









	class VulkanTexture2D : public VulkanImage, public ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanTexture2D> make(VulkanDevice& device, uint32 width, uint32 height,
			uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);

		void createImageView();

	private:
		VulkanTexture2D(VulkanDevice& device, uint32 width, uint32 height,
			uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);
		VulkanTexture2D(const VulkanTexture2D&) = delete;
		VulkanTexture2D& operator=(const VulkanTexture2D&) = delete;
	};










	class VulkanTextureCube : public VulkanImage, ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanTextureCube> make(VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			uint32 mipLevels = 1, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);

		void createImageView();

	private:
		VulkanTextureCube(VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			uint32 mipLevels = 1, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);
		VulkanTextureCube(const VulkanTextureCube&) = delete;
		VulkanTextureCube& operator=(const VulkanTextureCube&) = delete;
	};










	class VulkanDepthStencilImage : public VulkanImage, ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanDepthStencilImage> make(VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);

		void createImageView();

	private:
		VulkanDepthStencilImage(VulkanDevice& device, uint32 width, uint32 height, vk::Format format, 
			vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);
		VulkanDepthStencilImage(const VulkanDepthStencilImage&) = delete;
		VulkanDepthStencilImage& operator=(const VulkanDepthStencilImage&) = delete;
	};

	// TODO: add depth stencil array









	class VulkanColorAttachmentImage : public VulkanImage, ng::MakeConstructed {
	public:

		static std::shared_ptr<VulkanColorAttachmentImage> make(VulkanDevice& device, uint32 width, uint32 height,
			vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);

		void createImageView();

	private:
		VulkanColorAttachmentImage(VulkanDevice& device, uint32 width, uint32 height, 
			vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);
		VulkanColorAttachmentImage(const VulkanColorAttachmentImage&) = delete;
		VulkanColorAttachmentImage& operator=(const VulkanColorAttachmentImage&) = delete;
	};

	// TODO: add color attachment array










	class VulkanSamplerMaker {
	public:

		VulkanSamplerMaker();

		VulkanSamplerMaker& flags(vk::SamplerCreateFlags value);

		VulkanSamplerMaker& magFilter(vk::Filter filter);

		VulkanSamplerMaker& minFilter(vk::Filter filter);

		VulkanSamplerMaker& mipmapMode(vk::SamplerMipmapMode value);
		VulkanSamplerMaker& addressModeU(vk::SamplerAddressMode value);
		VulkanSamplerMaker& addressModeV(vk::SamplerAddressMode value);
		VulkanSamplerMaker& addressModeW(vk::SamplerAddressMode value);
		VulkanSamplerMaker& mipLodBias(float value);
		VulkanSamplerMaker& anisotropyEnable(vk::Bool32 value);
		VulkanSamplerMaker& maxAnisotropy(float value);
		VulkanSamplerMaker& compareEnable(vk::Bool32 value);
		VulkanSamplerMaker& compareOp(vk::CompareOp value);
		VulkanSamplerMaker& minLod(float value);
		VulkanSamplerMaker& maxLod(float value);
		VulkanSamplerMaker& borderColor(vk::BorderColor value);
		VulkanSamplerMaker& unnormalizedCoordinates(vk::Bool32 value);
		
		vk::Sampler create(vk::Device device) const;

		vk::UniqueSampler createUnique(vk::Device device) const;


	private:
		vk::SamplerCreateInfo m_Info;
	};




}