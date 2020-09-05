#pragma once


#include "../ng_utility.h"
#include "pch.h"


namespace ngv {


	//Forward declarations
	class VulkanAllocator;
	class VulkanMemoryPage;
	class VulkanMemoryAllocation;	
	class VulkanDevice;


	// Base Buffer Class
	MAKE_CONSTRUCTED
	class VulkanBuffer {
	public:

		static std::unique_ptr<VulkanBuffer> Make(const VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer = false);

		~VulkanBuffer() = default; // allocations should be RAII deallocated
		
		vk::Buffer Buffer() const;
		const vk::BufferCreateInfo GetBufferCreateInfo() const;
		const vk::MemoryPropertyFlags GetMemoryPropertyFlags() const;

		void* Map();
		void* Map(uint64 offset, uint64 size);
		void Unmap();


		void UpdateLocal(const void* value, vk::DeviceSize size) const;

		template<class T, class Allocator>
		void UpdateLocal(const std::vector<T, Allocator>& values) const;

		template<class T>
		void UpdateLocal(const T& value) const;



		void Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const void* value, vk::DeviceSize size);
		
		template<typename T>
		void Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, std::vector<T>& values);
		
		template<typename T>
		void Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const T& value);


		void Barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
			vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, 
			uint32 srcQueueFamilyIndex, uint32 dstQueueFamilyIndex) const;

		void Flush();

		void Invalidate();

		bool HasAllocation();
		bool HasSameAllocation(const VulkanBuffer& buffer);

	protected:

		void create(const VulkanDevice& device, 
			const vk::BufferCreateInfo& info, bool hostBuffer = false);

	protected:
		VulkanBuffer() = default;
		VulkanBuffer(const VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer = false);
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

		std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;

	};

	template<class T, class Allocator>
	inline void VulkanBuffer::UpdateLocal(const std::vector<T, Allocator>& values) const
	{
		UpdateLocal( (void*)values.data(), (vk::DeviceSize)values.size() * sizeof(T) );
	}

	template<class T>
	inline void VulkanBuffer::UpdateLocal(const T& value) const
	{
		UpdateLocal((void*)&value, vk::DeviceSize(sizeof(T)));
	}

	template<typename T>
	inline void VulkanBuffer::Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, std::vector<T>& values)
	{
		Upload(cb, stagingBuffer, (void*)values.data(), (uint64)values.size());
	}

	template<typename T>
	inline void VulkanBuffer::Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const T& value)
	{
		Upload(cb, stagingBuffer, (void*)&value, sizeof(value));
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
	MAKE_CONSTRUCTED
	class VulkanImage {
	public:

		static std::unique_ptr<VulkanImage> Make(const VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage = false);

		~VulkanImage() = default;

		void CreateImageView(vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask);

		// GETTERS
		vk::Image Image() const;
		vk::ImageView ImageView() const;
		//vk::DeviceMemory mem() const;
		vk::Format Format() const;
		vk::Extent3D Extent() const;
		const vk::ImageCreateInfo ImageCreateInfo() const;
		const vk::MemoryPropertyFlags MemoryPropertyFlags() const;

		void Clear(vk::CommandBuffer cb, const std::array<float, 4> color = { 1,1,1,1 });


		void Copy(vk::CommandBuffer cb, ngv::VulkanImage& srcImage);

		void Copy(vk::CommandBuffer cb, vk::Buffer buffer, uint32 mipLevel,
			uint32 arrayLayer, uint32 width, uint32 height, uint32 depth, uint32 offset);


		void Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const void* value, vk::DeviceSize size);

		template<typename T>
		void Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, std::vector<T>& values);

		template<typename T>
		void Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const T& value);

		void SetLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

		/// Set what the image thinks is its current layout (ie. the old layout in an image barrier).
		void SetCurrentLayout(vk::ImageLayout oldLayout);

		vk::ImageLayout GetImageLayout();
		void SetImageLayout(vk::ImageLayout imageLayout);

		bool HasAllocation();
		bool HasSameAllocation(const VulkanImage& image);

	protected:

		void create(const VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage = false);

	protected:
		VulkanImage() = default;
		VulkanImage(const VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage = false);
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;

	private:
		friend class VulkanAllocator;

		vk::ImageCreateInfo m_ImageCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;
		vk::ImageLayout m_CurrentLayout = vk::ImageLayout::eUndefined;

		vk::UniqueImage m_Image;
		vk::UniqueImageView m_ImageView;
		//vk::ImageLayout m_ImageLayout;
		vk::MemoryRequirements m_MemoryRequirements;
		uint32 m_MemoryTypeIndex = 0;

		bool m_Created = false;

		std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;
	};

	template<typename T>
	inline void VulkanImage::Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, std::vector<T>& values)
	{
		Upload(cb, stagingBuffer, (void*)values.data(), (uint32)values.size());
	}

	template<typename T>
	inline void VulkanImage::Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const T& value)
	{
		Upload(cb, stagingBuffer, (void*)&value, sizeof(T));
	}









	class VulkanVertexBuffer : public VulkanBuffer {
	public:

		static std::unique_ptr<VulkanVertexBuffer> Make(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VulkanVertexBuffer(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VulkanVertexBuffer(const VulkanVertexBuffer&) = delete;
		VulkanVertexBuffer& operator=(const VulkanVertexBuffer&) = delete;
	};











	class VulkanIndexBuffer : public VulkanBuffer {
	public:

		static std::unique_ptr<VulkanIndexBuffer> Make(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VulkanIndexBuffer(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VulkanIndexBuffer(const VulkanIndexBuffer&) = delete;
		VulkanIndexBuffer& operator=(const VulkanIndexBuffer&) = delete;
	}; 









	class VulkanUniformBuffer : public VulkanBuffer {
	public:

		static std::unique_ptr<VulkanUniformBuffer> Make(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VulkanUniformBuffer(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VulkanUniformBuffer(const VulkanUniformBuffer&) = delete;
		VulkanUniformBuffer& operator=(const VulkanUniformBuffer&) = delete;
	};









	class VulkanTexture2D : public VulkanImage {
	public:

		static std::unique_ptr<VulkanTexture2D> Make(const VulkanDevice& device, uint32 width, uint32 height,
			uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);

		void CreateImageView();

	private:
		VulkanTexture2D(const VulkanDevice& device, uint32 width, uint32 height,
			uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);
		VulkanTexture2D(const VulkanTexture2D&) = delete;
		VulkanTexture2D& operator=(const VulkanTexture2D&) = delete;
	};










	class VulkanTextureCube : public VulkanImage {
	public:

		static std::unique_ptr<VulkanTextureCube> Make(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			uint32 mipLevels = 1, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);

		void CreateImageView();

	private:
		VulkanTextureCube(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			uint32 mipLevels = 1, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);
		VulkanTextureCube(const VulkanTextureCube&) = delete;
		VulkanTextureCube& operator=(const VulkanTextureCube&) = delete;
	};










	class VulkanDepthStencilImage : public VulkanImage {
	public:

		static std::unique_ptr<VulkanDepthStencilImage> Make(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);

		void CreateImageView();

	private:
		VulkanDepthStencilImage(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format, 
			vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);
		VulkanDepthStencilImage(const VulkanDepthStencilImage&) = delete;
		VulkanDepthStencilImage& operator=(const VulkanDepthStencilImage&) = delete;
	};

	// TODO: add depth stencil array









	class VulkanColorAttachmentImage : public VulkanImage {
	public:

		static std::unique_ptr<VulkanColorAttachmentImage> Make(const VulkanDevice& device, uint32 width, uint32 height,
			vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);

		void CreateImageView();

	private:
		VulkanColorAttachmentImage(const VulkanDevice& device, uint32 width, uint32 height, 
			vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);
		VulkanColorAttachmentImage(const VulkanColorAttachmentImage&) = delete;
		VulkanColorAttachmentImage& operator=(const VulkanColorAttachmentImage&) = delete;
	};

	// TODO: add color attachment array



	// RAY TRACING STUFF

	class VulkanAccelerationStructure {
	public:

	private:
		vk::AccelerationStructureKHR m_AccelerationStructure;
		uint64 m_Handle;
		
		uint64 m_DeviceAddress = 0;

		std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;

	};











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