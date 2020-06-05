#pragma once

#include "../def.h"
#include "vulkandef.h"

#include <list>

#include <gli/texture2d.hpp>
#include <utility>

namespace ngv {


	/*  TEXTURES  */


	class VulkanAllocator;
	class VulkanMemoryPage;
	class VulkanMemoryAllocation;

	struct VulkanImageCreateInfo {
		vk::ImageCreateInfo imageCreateInfo;
		vk::MemoryPropertyFlags memoryPropertyFlags;
		vk::ImageViewType viewType;
	};

	struct ImageBlockParams {
		uint8 blockWidth;
		uint8 blockHeight;
		uint8 bytesPerBlock;
	};

	class VulkanImage {
	public:

		static std::unique_ptr<VulkanImage> make();

		~VulkanImage() = default;

		static ImageBlockParams getBlockParams(vk::Format format);

		static uint32 mipScale(uint32 value, uint32 mipLevel);

		vk::ImageCreateInfo ngv::VulkanImage::getImageCreateInfo();

		vk::MemoryPropertyFlags ngv::VulkanImage::getMemoryPropertyFlags();

	private:

	private:
		friend class VulkanAllocator;
		
		vk::ImageCreateInfo m_ImageCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;


		vk::ImageViewType m_ViewType;



	};

	struct VulkanBufferCreateInfo {
		vk::BufferCreateInfo bufferCreateInfo;
		vk::MemoryPropertyFlags memoryPropertyFlags;
	};

	class VulkanBuffer : public ng::MakeConstructed<VulkanBuffer> {
	public:

		static std::unique_ptr<VulkanBuffer> make();

		~VulkanBuffer() = default; // allocations should be RAII deallocated

		vk::BufferCreateInfo getBufferCreateInfo();

		vk::MemoryPropertyFlags getMemoryPropertyFlags();

		bool hasAllocation();

		bool hasSameAllocation(std::raw_ptr<VulkanBuffer> buffer);

		bool swapAllocation(VulkanBuffer& withAllocation);

	private:
		VulkanBuffer() = default;
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

	private:
		friend class VulkanAllocator;
		vk::BufferCreateInfo m_BufferCreateInfo;
		vk::MemoryPropertyFlags m_MemoryPropertyFlags;

		vk::UniqueBuffer m_Buffer;
		vk::MemoryRequirements m_MemoryRequirements;
		uint32 m_MemoryTypeIndex;


		std::unique_ptr<VulkanMemoryAllocation> m_pAllocation;
		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;

	};
}

