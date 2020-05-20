#pragma once

#include "vulkandef.h"
#include "vulkan_allocator.h"

#include <list>

#include <gli/texture2d.hpp>
#include <utility>

namespace ngv {


	/*  TEXTURES  */

	class VulkanImageAllocation;

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

		VulkanImage();

		VulkanImage(VulkanImage& image);

		~VulkanImage();

		vk::ImageCreateInfo getImageCreateInfo();

		vk::MemoryPropertyFlags getMemoryPropertyFlags();

		bool hasAllocation();

		bool hasSameAllocation(VulkanImage& image);

		/* Swap two allocations */
		bool swapAllocation(VulkanImage& withAllocation);

		/* Uploads the image-data to the allocation, note that this will effect every image that references the same allocation*/
		bool upload(vk::CommandBuffer, const VulkanAllocator& allocator, const void* data, const vk::ImageLayout layout);



		static ImageBlockParams getBlockParams(vk::Format format);

		static uint32 mipScale(uint32 value, uint32 mipLevel);

	private:

	private:
		friend class VulkanAllocator;

		VulkanImageCreateInfo m_CreateInfo;

		std::weak_ptr<VulkanImageAllocation> m_pAllocation;

		std::shared_ptr<std::set<VulkanImage*>> m_pImages;


	};


	class VulkanBufferAllocation;

	struct VulkanBufferCreateInfo {
		vk::BufferCreateInfo bufferCreateInfo;
		vk::MemoryPropertyFlags memoryPropertyFlags;
	};

	class VulkanBuffer {
	public:

		VulkanBuffer();

		VulkanBuffer(VulkanBuffer& buffer);

		~VulkanBuffer();

		vk::BufferCreateInfo getBufferCreateInfo();

		vk::MemoryPropertyFlags getMemoryPropertyFlags();

		bool hasAllocation();

		bool hasSameAllocation(VulkanBuffer& buffer);

		bool swapAllocation(VulkanBuffer& withAllocation);

	private:

	private:
		friend class VulkanAllocator;

		VulkanBufferCreateInfo m_CreateInfo;

		std::weak_ptr<VulkanBufferAllocation> m_pAllocation;

		std::shared_ptr<std::set<VulkanBuffer*>> m_pBuffers;

	};
}

