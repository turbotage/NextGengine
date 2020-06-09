#pragma once

#include "../def.h"
#include "vulkandef.h"

#include <list>

#include <gli/texture2d.hpp>
#include <utility>
#include <vector>

namespace ngv {


	//Forward declarations
	class VulkanAllocator;
	class VulkanMemoryPage;
	class VulkanMemoryAllocation;	
	class VulkanDevice;


	// Base Buffer Class
	class VulkanBuffer : public ng::MakeConstructed<VulkanBuffer> {
	public:

		static std::unique_ptr<VulkanBuffer> make(VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer = false);

		~VulkanBuffer() = default; // allocations should be RAII deallocated
		
		vk::Buffer buffer() const;
		const vk::BufferCreateInfo getBufferCreateInfo() const;
		const vk::MemoryPropertyFlags getMemoryPropertyFlags() const;


		void* map();
		void unmap();


		bool updateLocal(const void* value, vk::DeviceSize size) const;

		template<class Type, class Allocator>
		bool updateLocal(const std::vector<Type, Allocator>& value) const;

		template<class Type>
		bool updateLocal(const Type& value);



		bool upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const void* value, vk::DeviceSize size);
		
		template<typename T>
		bool upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values);
		
		template<typename T>
		bool upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value);


		void barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
			vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, 
			uint32 srcQueueFamilyIndex, uint32 dstQueueFamilyIndex) const;

		bool flush(const VulkanDevice& device);

		bool invalidate(const VulkanDevice& device);

		bool hasAllocation();
		bool hasSameAllocation(const VulkanBuffer& buffer);

	protected:

		void create(ngv::VulkanDevice& device, 
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

		std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;
		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;

	};

	template<class Type, class Allocator>
	inline bool VulkanBuffer::updateLocal(const std::vector<Type, Allocator>& value) const
	{
		return updateLocal((void*)value.data(), vk::DeviceSize(sizeof(Type)));
	}

	template<class Type>
	inline bool VulkanBuffer::updateLocal(const Type& value)
	{
		return updateLocal((void*)&value, vk::DeviceSize(sizeof(Type)));
	}

	template<typename T>
	inline bool VulkanBuffer::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values)
	{
		return upload(cb, stagingBuffer, (void*)values.data(), (uint64)values.size());
	}

	template<typename T>
	inline bool VulkanBuffer::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value)
	{
		return upload(cb, stagingBuffer, (void*)&value, sizeof(value));
	}







	class VulkanSparseBuffer : public ng::MakeConstructed<VulkanSparseBuffer> {
	public:

	private:
		friend class VulkanAllocator;



		vk::BufferCreateInfo m_BufferCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;

		vk::UniqueBuffer m_Buffer;

		vk::BindSparseInfo m_BindInfo;

	};







	// Base Image Class
	class VulkanImage : public ng::MakeConstructed<VulkanImage> {
	public:

		static std::unique_ptr<VulkanImage> make(VulkanDevice& device, const vk::ImageCreateInfo& info,
			vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask, bool hostImage = false);

		~VulkanImage() = default;

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
		bool upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values);

		template<typename T>
		bool upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value);

		void setLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

		/// Set what the image thinks is its current layout (ie. the old layout in an image barrier).
		void setCurrentLayout(vk::ImageLayout oldLayout);

		bool hasAllocation();
		bool hasSameAllocation(const VulkanImage& image);

	protected:

		void create(VulkanDevice& device, const vk::ImageCreateInfo& info,
			vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask, bool hostImage = false);

	protected:
		VulkanImage() = default;
		VulkanImage(VulkanDevice& device, const vk::ImageCreateInfo& info,
			vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask, bool hostImage = false);
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

		std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;
		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;
	};

	template<typename T>
	inline bool VulkanImage::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, std::vector<T>& values)
	{
		return upload(cb, stagingBuffer, (void*)values.data(), (uint32)values.size());
	}

	template<typename T>
	inline bool VulkanImage::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const T& value)
	{
		return upload(cb, stagingBuffer, (void*)&value, sizeof(T));
	}









	class VertexBuffer : public VulkanBuffer, public ng::MakeConstructed<VertexBuffer> {
	public:

		static std::unique_ptr<VertexBuffer> make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		VertexBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;
	};











	class IndexBuffer : public VulkanBuffer, public ng::MakeConstructed<IndexBuffer> {
	public:

		static std::unique_ptr<IndexBuffer> make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		IndexBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;
	}; 









	class UniformBuffer : public VulkanBuffer, public ng::MakeConstructed<UniformBuffer> {
	public:

		static std::unique_ptr<UniformBuffer> make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);

	private:
		UniformBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer = false);
		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;
	};









	class Texture2D : public VulkanImage, public ng::MakeConstructed<Texture2D> {
	public:

		std::unique_ptr<Texture2D> make(VulkanDevice& device, uint32 width, uint32 height, 
			uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);

	private:
		Texture2D(VulkanDevice& device, uint32 width, uint32 height,
			uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
	};












	class TextureCube : public VulkanImage, ng::MakeConstructed<TextureCube> {
	public:

		std::unique_ptr<TextureCube> make(VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			uint32 mipLevels = 1, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);

	private:
		TextureCube(VulkanDevice& device, uint32 width, uint32 height, vk::Format format,
			uint32 mipLevels = 1, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1, bool hostImage = false);
		TextureCube(const TextureCube&) = delete;
		TextureCube& operator=(const TextureCube&) = delete;
	};











	class DepthStencilImage : public VulkanImage, ng::MakeConstructed<DepthStencilImage> {
	public:

		std::unique_ptr<DepthStencilImage> make(VulkanDevice& device, uint32 width, uint32 height, vk::Format format, 
			vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);

	private:
		DepthStencilImage(VulkanDevice& device, uint32 width, uint32 height, vk::Format format, 
			vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);
		DepthStencilImage(const DepthStencilImage&) = delete;
		DepthStencilImage& operator=(const DepthStencilImage&) = delete;
	};

	// TODO: add depth stencil array










	class ColorAttachmentImage : public VulkanImage, ng::MakeConstructed<ColorAttachmentImage> {
	public:

		std::unique_ptr<ColorAttachmentImage> make(VulkanDevice& device, uint32 width, uint32 height,
			vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);

	private:
		ColorAttachmentImage(VulkanDevice& device, uint32 width, uint32 height, 
			vk::Format format, vk::SampleCountFlagBits sampleFlags = vk::SampleCountFlagBits::e1);
		ColorAttachmentImage(const ColorAttachmentImage&) = delete;
		ColorAttachmentImage& operator=(const ColorAttachmentImage&) = delete;
	};

	// TODO: add color attachment array










	class SamplerMaker {
	public:

		SamplerMaker();

		SamplerMaker& flags(vk::SamplerCreateFlags value);

		SamplerMaker& magFilter(vk::Filter filter);

		SamplerMaker& minFilter(vk::Filter filter);

		SamplerMaker& mipmapMode(vk::SamplerMipmapMode value);
		SamplerMaker& addressModeU(vk::SamplerAddressMode value);
		SamplerMaker& addressModeV(vk::SamplerAddressMode value);
		SamplerMaker& addressModeW(vk::SamplerAddressMode value);
		SamplerMaker& mipLodBias(float value);
		SamplerMaker& anisotropyEnable(vk::Bool32 value);
		SamplerMaker& maxAnisotropy(float value);
		SamplerMaker& compareEnable(vk::Bool32 value);
		SamplerMaker& compareOp(vk::CompareOp value);
		SamplerMaker& minLod(float value);
		SamplerMaker& maxLod(float value);
		SamplerMaker& borderColor(vk::BorderColor value);
		SamplerMaker& unnormalizedCoordinates(vk::Bool32 value);
		
		vk::Sampler create(vk::Device device) const;

		vk::UniqueSampler createUnique(vk::Device device) const;


	private:
		vk::SamplerCreateInfo m_Info;
	};




}