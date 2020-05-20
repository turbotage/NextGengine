#pragma once


#include "vulkandef.h"
#include "vulkan_context.h"
#include "vulkan_storage.h"

#include "vulkan_abstract_allocators.h"

#include <list>
#include <map>

/* <==================== VULKAN MEMORY ALLOCATOR =================>  */

namespace ngv {

	//forward declarations
	struct VulkanMemoryPage;

	struct VulkanImageCreateInfo;
	class VulkanImage;
	class VulkanImagePage;

	struct VulkanBufferCreateInfo;
	class VulkanBuffer;
	class VulkanBufferPage;

	struct VulkanUniformBufferCreateInfo;
	class VulkanUniformBuffer;
	class VulkanUniformBufferPage;


	struct VulkanMemoryStrategy {
		/*  The largest allocation size for the allocation to not be given a own dedicated memory block  */
		vk::DeviceSize largestNonSingleAllocationSize = 64 * 1024 * 1024;

		/*  Free Allocator Settings  */
		vk::DeviceSize freeAllocatorBlockSize = 1024;

		/*  Uniform Buffer Settings*/
		vk::DeviceSize uniformBufferAlignment = 16;


		vk::DeviceSize largeCapImageSize = 16 * 1024 * 1024;
		uint32 largeCapImagesPerPool = 8;

		vk::DeviceSize mediumCapImageSize = 4 * 1024 * 1024;
		uint32 mediumCapImagesPerPool = 32;

		vk::DeviceSize smallCapImageSize = 256 * 1024;
		uint32 smallCapImagesPerPool = 32;


		vk::DeviceSize bufferSize = 128 * 1024 * 1024;


	};

	struct VulkanAllocatorHints {
		// type of image, number of similar images
		std::vector<std::tuple<vk::ImageCreateInfo, vk::MemoryPropertyFlags, uint32>> imageHints;

	};

	struct VulkanAllocatorCreateInfo {
		VulkanMemoryStrategy memoryStrategy;
	};

	class VulkanAllocator {
	public:

		void init(VulkanAllocatorCreateInfo createInfo);

		/* Long time hints, a strategy to help the allocator not needing to micromanage memory as much every frame */
		bool giveAllocatorHints(VulkanAllocatorHints hints);

		void memoryManage(vk::CommandBuffer cmdBuffer);


		/*  IMAGES  */
		bool giveImagesAllocations(const std::vector<std::reference_wrapper<VulkanImage>>& images);
		/*  Used when the iamge has never been initialized before or when it should be re-initialized  */
		bool createImage(const VulkanImageCreateInfo& createInfo, VulkanImage& image, bool allocate = true);
		/*  Used when the iamge has been initialized before but lacks allocation and we wan't to find a allocation for it */
		bool giveImageAllocation(VulkanImage& image);


		/*  BUFFERS  */
		bool giveBuffersAllocations(const std::vector<std::reference_wrapper<VulkanBuffer>>& buffers);
		/*  Used when the buffer has never been initialized before  */
		bool createBuffer(const VulkanBufferCreateInfo& createInfo, VulkanBuffer& buffer, bool allocate = true);
		/*  Used when the buffer has been initialized before but lacks allocation and we wan't to find a allocation for it */
		bool giveBufferAllocation(VulkanBuffer& buffer);


		/*  UNIFORM-BUFFERS  */
		bool giveUniformBuffersAllocations(const std::vector <std::reference_wrapper<VulkanUniformBuffer>>& uniformBuffers);
		/*  Used when the buffer has never been initialized before  */
		bool createUniformBuffer(const VulkanUniformBufferCreateInfo& createInfo, VulkanUniformBuffer& uniformBuffer, bool allocate = true);
		/*  Used when the uniform-buffer has been initialized before but lacks allocation and we wan't to find a allocation for it */
		bool giveUniformBufferAllocation(VulkanUniformBuffer& buffer);

	public:

		vk::Device getDevice();

		static vk::DeviceSize getAlignedSize(vk::DeviceSize size, vk::DeviceSize alignment);

	private:

		/* <--------- CREATE MEMORY PAGES ----------> */
		std::list<VulkanImagePage>::iterator createImagePage(const VulkanImageCreateInfo& createInfo, 
			std::shared_ptr<std::list<VulkanImagePage>> pImageTypeList, vk::Result& result, bool inDevice);

		std::list<VulkanBufferPage>::iterator createBufferPage(const VulkanBufferCreateInfo& createInfo, vk::Result& result, bool inDevice);

		std::list<VulkanUniformBufferPage>::iterator createUniformBufferPage(const VulkanUniformBufferCreateInfo& createInfo, 
			std::shared_ptr<std::list<VulkanUniformBufferPage>> pUniformBufferTypeList, vk::Result& result, bool inDevice);

		/* <--------- FIND ITERATORS -----------> */
		std::list<VulkanImagePage>::iterator findImagePage(const VulkanImageCreateInfo& createInfo, 
			std::shared_ptr<std::list<VulkanImagePage>> pPageList, vk::Result& result);

		std::list<VulkanBufferPage>::iterator findBufferPage(const VulkanBufferCreateInfo& createInfo, vk::Result& result);

		std::list<VulkanUniformBufferPage>::iterator findUniformBufferPage(const VulkanUniformBufferCreateInfo& createInfo,
			std::shared_ptr<std::list<VulkanUniformBufferPage>> pPageList, vk::Result& result);

		/* <--------- HELP FUNCTIONS ---------------> */

	private:

		bool unifiedTransferAndGraphicsQueue;

		std::shared_ptr<VulkanContext> m_Context;
		vk::Device m_Device;

		VulkanMemoryStrategy m_MemoryStrategy;

		bool hasMemoryForHints;
		std::vector<std::tuple<vk::ImageCreateInfo, vk::MemoryPropertyFlags, uint32>> m_CurrentImageHints;
		std::vector<uint32> m_AvailableHintImages;

		/* <------------- IMAGES -------------> */
		//device local
		std::list<std::shared_ptr<std::list<VulkanImagePage>>> m_DeviceImagePageLists;
		//host local
		std::list<std::shared_ptr<std::list<VulkanImagePage>>> m_HostImagePageLists;

		/* <------------- BUFFER -------------> */
		//device local
		std::list<VulkanBufferPage> m_DeviceBufferPages;
		//host local
		std::list<VulkanBufferPage> m_HostBufferPages;
		//list of pointers to staging buffers


		/* <------------- UNIFORM-BUFFER -------------> */
		//device local
		std::list<std::shared_ptr<std::list<VulkanUniformBufferPage>>> m_DeviceUniformBufferPageLists;
		//host local
		std::list<std::shared_ptr<std::list<VulkanUniformBufferPage>>> m_HostUniformBufferPageLists;

	};

}




/* <======================== MEMORY PAGE TYPES =======================>*/

namespace ngv {

	class VulkanPoolAllocator;
	class VulkanFreeAllocator;


	class VulkanMemoryPage {
	public:

		VulkanMemoryPage();

		~VulkanMemoryPage();

	public:
		//Memory
		vk::UniqueDeviceMemory memory;
		vk::DeviceSize size;

		vk::MemoryPropertyFlags memoryPropertyFlags;
		uint32 memoryTypeIndex;
	};


	/* <------------ Image Allocation and Page -------------------> */
	class VulkanImageAllocation {
	public:

		VulkanImageAllocation();

		VulkanImagePage* pImagePage;

		VulkanPoolAllocation poolAllocation;

		std::set<VulkanImage*> images;

		vk::UniqueImage image;
		vk::UniqueImageView imageView;

	};

	class VulkanImagePage {
	public:
		VulkanImagePage();

		bool hasFreeAllocations();

		bool isSuitable(const VulkanImageCreateInfo& createInfo);

		std::shared_ptr<ngv::VulkanImageAllocation> getAllocation();

		void freeAllocation(std::shared_ptr<ngv::VulkanImageAllocation> toFree);

		uint32 getNumFreeAllocations();

	public:
		friend class VulkanAllocator;

		VulkanAllocator* pVulkanAllocator;

		VulkanPoolAllocator poolAllocator;
		
		std::shared_ptr<VulkanMemoryPage> memoryPage;
		// the specifications for the images that are to be allocated from this page
		vk::ImageCreateInfo imageCreateInfo;
		// memory requirements
		vk::MemoryRequirements memoryRequirements;


		std::set<std::shared_ptr<ngv::VulkanImageAllocation>> freeImageAllocations;
		std::set<std::shared_ptr<ngv::VulkanImageAllocation>> usedImageAllocations;

	};


	/* <----------------------- Buffer Allocation and Page ------------------------> */
	class VulkanBufferAllocation {
	public:
		VulkanBufferAllocation();

		VulkanBufferPage* pBufferPage;

		VulkanFreeAllocation freeAllocations;

		std::set<VulkanBuffer*> buffers;

		vk::UniqueBuffer buffer;

	};

	class VulkanBufferPage {
	public:
		VulkanBufferPage();

		bool isSuitable(const VulkanBufferCreateInfo& createInfo);

		bool enoughSpaceWithoutDefrag(vk::DeviceSize requiredSize);

		bool enoughSpace(vk::DeviceSize requiredSize);

	public:
		std::shared_ptr<VulkanMemoryPage> memoryPage;

		VulkanFreeAllocator allocator;

		vk::BufferCreateFlags createFlags;
		vk::BufferUsageFlags usageFlags;

		vk::UniqueBuffer buffer;


	};


	/* <----------------------- Uniform Buffer Allocation and Page ------------------------> */
	class VulkanUniformBufferPage {
	public:
		VulkanUniformBufferPage();
	public:
		std::shared_ptr<VulkanMemoryPage> memoryPage;

		VulkanPoolAllocator allocator;

		VkBufferUsageFlags usageFlags;

		vk::UniqueBuffer buffer;


	};


}


