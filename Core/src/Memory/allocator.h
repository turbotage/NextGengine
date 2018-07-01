#pragma once

#include "../def.h"
#include <vector>
#include <mutex>
#include <map>
#include <unordered_map>
#include "../Vulkan/vulkan_device.h"
#include "../debug.h"

#define MEMORY_ALIGNMENT 16

//allocator should largely be copied for textureswzs

struct Block {
	VkDeviceSize offset;
	VkDeviceSize size;

	static bool equalOffset(const Block& left, const Block& right) {
		return left.offset == right.offset;
	}

	static bool equalOffsetPlusSize(const Block& left, const Block& right) {
		return (left.offset + left.size) == (right.offset + right.size);
	}

	static bool equalOffsetPlusSizeAsOffset(const Block& left, const VkDeviceSize& right) {
		return (left.offset + left.size) == right;
	}

};

struct VulkanAllocationCreateInfo {
	VkDeviceSize size;
	VkDeviceSize dataSize;
};

struct VulkanBufferCreateInfo {
	VkDeviceSize size;
	void* data;
	VkDeviceSize dataSize;
};

class VulkanAllocation {
public:
	VkDeviceSize offset;
	VkDeviceSize size;
	VkDeviceSize dataSize;

	static bool offsetSmallerThan(const VkDeviceSize& offset, const VulkanAllocation& block) {
		return offset < block.offset;
	}

	static bool sizeSmallerThan(const VkDeviceSize& size, const VulkanAllocation& block) {
		return size < block.size;
	}

};

class VulkanBuffer {
private:
	friend class VulkanBufferAllocator;
	VulkanBufferAllocator* m_BufferAllocator;

	//should always
	VulkanBufferAllocator::VulkanMemoryChunk* m_StagingChunk = nullptr;
	std::shared_ptr<VulkanAllocation> m_StagingAllocation = nullptr;

	VulkanBufferAllocator::VulkanMemoryChunk* m_DeviceChunk = nullptr;
	std::shared_ptr<VulkanAllocation> m_Allocation = nullptr;

public:


	/* will return false if this is host local memory only */
	bool moveToDevice() {

	}

	bool free() {
		if (m_Allocation != nullptr) {
			if (m_Allocation.use_count() == 1) {
				m_DeviceChunk->free(*m_Allocation);
			}
		}
		m_Allocation.reset();
		if (m_StagingAllocation.use_count() == 1) {
			m_StagingChunk->free(*m_StagingAllocation);
		}
		m_StagingAllocation.reset();
	}

};


/*
void createBuffer(VulkanBufferCreateInfo createInfo, VulkanBuffer& buffer) :
	copy the data from createInfo.data to allocation in staging buffer, 
*/
class VulkanBufferAllocator
{
private:

	friend class VulkanBuffer;

	class VulkanMemoryChunk
	{
	public:
		std::atomic<bool> hasFailedToFindMatch; //will be reset after defragmentation
		std::atomic<VkDeviceSize> lastFailedAllocateSize; //will be reset after defragmentation

		VkDeviceMemory memory;
		VkDeviceSize size;
		VkBuffer buffer;

		VkDeviceSize totalFreeSpace;

		std::list<Block> freeBlocks; // not sorted at all
		std::list<VulkanAllocation> allocations; //sorted by offset

		/*    */
		VulkanMemoryChunk(VkDeviceSize size) {
			size = size;
		}

		VkResult create(ng::vulkan::VulkanDevice* vulkanDevice,
			VkMemoryPropertyFlags flags,
			VkBufferUsageFlags usage)
		{

			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult result = vkCreateBuffer(vulkanDevice->logicalDevice, &bufferInfo, nullptr, &buffer);
			if (result != VK_SUCCESS) {
				LOGD("failed to create chunk buffer");
				return result;
			}

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(vulkanDevice->logicalDevice, buffer, &memRequirements);

			VkBool32 memTypeFound;
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = vulkanDevice->getMemoryTypeIndex(memRequirements.memoryTypeBits, flags, &memTypeFound);

			result = vkAllocateMemory(vulkanDevice->logicalDevice, &allocInfo, nullptr, &memory);
			if (result != VK_SUCCESS) {
				LOGD("failed to create chunk memory");
				return result;
			}

			vkBindBufferMemory(vulkanDevice->logicalDevice, buffer, memory, 0);

			freeBlocks.emplace_front(0, size);
			totalFreeSpace = size;
		}

		VkDeviceSize getTotalFreeSpace() { return totalFreeSpace; }

		uint64 getNumFreeSpaces() { return freeBlocks.size(); }

		/* returns iterator to freeBlocks.end() if no element found */
		std::list<Block>::iterator getClosestMatch(VkDeviceSize size, bool lockMutex = true) {

			auto smallestIt = freeBlocks.end();

			//first find first block bigger than size
			for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
				if (it->size > size) {
					smallestIt = it;
					break;
				}
			}
			//start searching for the smallest one from the first found block
			for (auto it = ++smallestIt; it != freeBlocks.end(); ++it) {
				if (it->size > size) {
					if (it->size < smallestIt->size) {
						smallestIt = it;
					}
				}
			}

			return smallestIt;
		}

		/* createInfo.size must be correctly aligned */
		std::shared_ptr<VulkanAllocation> allocate(VulkanAllocationCreateInfo createInfo) {
			std::shared_ptr<VulkanAllocation> ret;

			auto freeBlockIt = getClosestMatch(createInfo.size, false);
			if (freeBlockIt == freeBlocks.end()) {
				/* this might happen with this method, shouldn't happen with other overloaded version of allocate since
				you have to give it the freespace to allocate from. Therefore the other allocate method should be  */
				hasFailedToFindMatch = true;
				lastFailedAllocateSize = createInfo.size;
				ret = nullptr;
			}

			//if freespace is in front or in back
			if ((freeBlockIt->offset + freeBlockIt->size == this->size) || (freeBlockIt->offset == 0)) {
				//case 1, freespace is in front
				if ((freeBlockIt->offset + freeBlockIt->size == this->size)) {
					ret = std::make_shared<VulkanAllocation>(&allocations.emplace_back(freeBlockIt->offset, createInfo.size, createInfo.dataSize));

					freeBlocks.emplace_front(ret->offset + ret->size, freeBlockIt->size - ret->size);

					totalFreeSpace = totalFreeSpace - freeBlockIt->size + ret->size;

					freeBlocks.erase(freeBlockIt);
					return ret;
				}
				//case 2, freespace is in back
				else {
					ret = std::make_shared<VulkanAllocation>(&allocations.emplace_front(freeBlockIt->offset, freeBlockIt->size, createInfo.dataSize));

					freeBlocks.emplace_front(ret->offset + ret->size, freeBlockIt->size - ret->size);

					totalFreeSpace = totalFreeSpace - freeBlockIt->size + ret->size;

					freeBlocks.erase(freeBlockIt);
					return ret;
				}
			}

			//case 3, freespace is somewhere between front and back
			for (auto it = allocations.begin(); it != allocations.end(); ++it) { //OBS now last iteration checks same thing as case 2
				if (freeBlockIt->offset == (it->offset + it->size)) {
					ret = std::make_shared<VulkanAllocation>(*allocations.emplace(it++, freeBlockIt->offset, freeBlockIt->size, createInfo.dataSize));

					freeBlocks.emplace_front(ret->offset + ret->size, freeBlockIt->size - ret->size);

					totalFreeSpace = totalFreeSpace - freeBlockIt->size + ret->size;

					freeBlocks.erase(freeBlockIt);
					return ret;
				}
			}

			LOGD("found suitable freespace but failed to find matchin allocation!");
			ng::debug::exitFatal("found suitable freespace but failed to find matchin allocation!", -1);
			//won't happen, written it anyway
			return ret;
		}

		/*  Will only create the new free block, won't erase the free block corrensponding to the freeBlock argument.
		Deletion of old freeSpace should be done after this function returns, will however change the var satoring the totalFreeSpace*/
		std::shared_ptr<VulkanAllocation> allocate(Block freeBlock, VulkanAllocationCreateInfo createInfo) {
			std::shared_ptr<VulkanAllocation> ret;

			//if freespace is in front or in back
			if ((freeBlock.offset + freeBlock.size == this->size) || (freeBlock.offset == 0)) {
				if ((freeBlock.offset + freeBlock.size == this->size)) {
					ret = std::make_shared<VulkanAllocation>(&allocations.emplace_back(freeBlock.offset, createInfo.size, createInfo.dataSize));

					freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

					totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

					return ret;
				}
				else {
					ret = std::make_shared<VulkanAllocation>(&allocations.emplace_front(freeBlock.offset, freeBlock.size, createInfo.dataSize));

					freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

					totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

					return ret;
				}
			}

			auto lastIt = allocations.begin();
			for (auto it = lastIt; it != allocations.end(); ++it) {
				if (freeBlock.offset == (it->offset + it->size)) {
					ret = std::make_shared<VulkanAllocation>(*allocations.emplace(it++, freeBlock.offset, freeBlock.size, createInfo.dataSize));

					freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

					totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

					return ret;
				}
			}

			LOGD("no allocation matched given freeBlock, not correct or non existing parameter?");
			hasFailedToFindMatch = true;
			lastFailedAllocateSize = createInfo.size;
			ret = nullptr;
			return ret;
		}

		void free(const VulkanAllocation& alloc) {

			bool hasLeftSpace = true;
			bool hasRightSpace = false;

			std::list<Block>::iterator freeBlockIt;

			Block leftFreeBlock = { 0,0 };
			if (alloc.offset != 0) {
				freeBlockIt = std::find_if(freeBlocks.begin(), freeBlocks.end(), std::bind(Block::equalOffsetPlusSizeAsOffset, std::placeholders::_1, alloc.offset));
				if (freeBlockIt != freeBlocks.end()) {
					hasLeftSpace = true;
					leftFreeBlock = *freeBlockIt;
					freeBlocks.erase(freeBlockIt);
				}
			}

			Block rightFreeBlock = { 0, 0 };
			if (alloc.offset + alloc.size != this->size) {
				rightFreeBlock = { alloc.offset + alloc.size, 0 };
				freeBlockIt = std::find_if(freeBlocks.begin(), freeBlocks.end(), std::bind(Block::equalOffset, std::placeholders::_1, rightFreeBlock));
				if (freeBlockIt != freeBlocks.end()) {
					hasRightSpace = true;
					rightFreeBlock = *freeBlockIt;
					freeBlocks.erase(freeBlockIt);
				}
			}

			Block newFreeBlock;

			if (hasLeftSpace) {
				//case 1
				if (hasRightSpace) {
					newFreeBlock = { leftFreeBlock.offset, leftFreeBlock.size + alloc.size + rightFreeBlock.size };
				}
				//case 2
				else {
					newFreeBlock = { leftFreeBlock.offset, leftFreeBlock.size + alloc.size };
				}
			}
			else if (hasRightSpace) {
				//case 3
				if (hasLeftSpace) {
					//do same as case 1
					newFreeBlock = { leftFreeBlock.offset, leftFreeBlock.size + alloc.size + rightFreeBlock.size };
				}
				//case 4
				else {
					newFreeBlock = { alloc.offset, alloc.size + rightFreeBlock.size };
				}
			}
			//case 5
			else {
				newFreeBlock = { alloc.offset, alloc.size };
			}

			allocations.erase(std::find(allocations.begin(), allocations.end(), alloc));
			freeBlocks.emplace_front(newFreeBlock.offset, newFreeBlock.size);

		}

		void defragment(ng::vulkan::VulkanDevice* vulkanDevice) {
			
		}

	};

private:

	ng::vulkan::VulkanDevice* m_VulkanDevice;

	VkDeviceSize m_StandardChunkSize;
	VkMemoryPropertyFlags m_MemoryFlags;
	VkMemoryAlignment m_MemoryAlignment;
	VkBufferUsageFlags m_BufferUsage;

	/*  When buffers should reside in device-local memory when used this are the vulkan-memory-chunks that holds the buffers for staging,
	the data will always be available here in the staging chunks to enable fast swapping. If the buffers should be non device-local, perhaps */
	std::forward_list<VulkanMemoryChunk> m_StagingChunks;

	/*  The vulkan-memory-chunks holding the buffers when they reside in device-local memory, 
	never used for host visible only buffers  */
	std::forward_list<VulkanMemoryChunk> m_DeviceChunks;

	std::forward_list<VulkanMemoryChunk>::iterator addChunk(std::forward_list<VulkanMemoryChunk>* chunks, VkResult* result = nullptr) {
		auto it = &chunks->emplace_front(m_StandardChunkSize, m_MemoryAlignment);
		VkResult res = it->create(m_VulkanDevice, m_MemoryFlags, m_BufferUsage);
		if (result != nullptr) {
			*result = res;
		}
	}

	VkDeviceSize getAlignedSize(VkDeviceSize size) {
		//should never be read and written to at the same time so no data race, no need to lock mutex
		if (size % m_MemoryAlignment != 0) {
			size = size + (m_MemoryAlignment - (size % m_MemoryAlignment));
		}
	}

	/* DeviceMemory : { Heap 0, MemoryType 0 }, StagingMemory : { Heap 2, MemoryType 2 } */
	VkResult createDeviceLocalWithStaging(VulkanBufferCreateInfo createInfo, VulkanBuffer& buffer) {
		bool hasStaging = false;
		bool inDeviceMemory = false;

		createInfo.size = getAlignedSize(createInfo.size);

		//staging-memory 
		for (int i = 0; i < 2 && !hasStaging; ++i) {
			for (auto& chunk : m_StagingChunks) {

				if (createInfo.size >= chunk.getTotalFreeSpace()) {
					if (createInfo.size != chunk.getTotalFreeSpace()) {
						continue;
					}
					else if (chunk.getNumFreeSpaces() != 1) {
						continue;
					}
				}

				auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

				if (freeBlockIt != chunk.freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_StagingAllocation = chunk.allocate(*freeBlockIt, allocInfo);
					buffer.m_StagingChunk = &chunk;

					//TODO write createInfo.data to stagingBuffer
					hasStaging = true;
					break;
				}

			}
			if (!hasStaging) {
				//if this is the second time we arives here the program can't continue from here
				if (i == 2) {
					LOGD("couldn't find staging memory for vulkanBuffer");
					ng::debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
				}
				VkResult result;
				auto it = addChunk(&m_StagingChunks, &result);
				if (result != VK_SUCCESS) {

					for (auto& chunk : m_StagingChunks) {
						if (chunk.getTotalFreeSpace() >= createInfo.size) {
							chunk.defragment(m_VulkanDevice);
						}
					}
					continue;
				}

				auto freeBlockIt = it->getClosestMatch(createInfo.size);

				if (freeBlockIt != it->freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
					buffer.m_StagingChunk = &*it;

					//TODO write createInfo.data to stagingBuffer
					hasStaging = true;
				}

				//should try to defrag chunks and then try again
				LOGD("failed to create Staging Buffer")
			}
		}

		//device-memory
		for (int i = 0; i < 2 && !hasStaging; ++i) {
			for (auto& chunk : m_DeviceChunks) {

				auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

				if (freeBlockIt != chunk.freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_Allocation = chunk.allocate(*freeBlockIt, allocInfo);
					buffer.m_DeviceChunk = &chunk;

					//TODO write data in stagingBuffer to deviceMemory
					inDeviceMemory = true;
					//done
					return;
				}

			}
			if (!inDeviceMemory) {
				//if this is the second time we arives here the program can't continue from here
				if (i == 2) {
					LOGD("couldn't find staging memory for vulkanBuffer");
					ng::debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
				}
				VkResult result;
				auto it = addChunk(&m_DeviceChunks, &result);
				if (result != VK_SUCCESS) {

					for (auto& chunk : m_StagingChunks) {
						if (chunk.getTotalFreeSpace() >= createInfo.size) {
							chunk.defragment(m_VulkanDevice);
						}
					}
					continue;
				}

				auto freeBlockIt = it->getClosestMatch(createInfo.size);

				if (freeBlockIt != it->freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
					buffer.m_StagingChunk = &*it;

					//TODO write createInfo.data to stagingBuffer
					hasStaging = true;
				}

				//should try to defrag chunks and then try again
				LOGD("failed to create Staging Buffer")
			}
		}

	}

	/* DeviceMemory : { Heap 1, MemoryType 1 }, StagingMemory : { Heap 2, MemoryType 2 }, 
	OBS! Does the same thing as createDeviceLocalWithStaging */
	VkResult createMappableDeviceLocal(VulkanBufferCreateInfo createInfo, VulkanBuffer& buffer) {
		bool hasStaging = false;
		bool inDeviceMemory = false;

		createInfo.size = getAlignedSize(createInfo.size);

		//staging-memory 
		for (int i = 0; i < 2 && !hasStaging; ++i) {
			for (auto& chunk : m_StagingChunks) {

				if (createInfo.size >= chunk.getTotalFreeSpace()) {
					if (createInfo.size != chunk.getTotalFreeSpace()) {
						continue;
					}
					else if (chunk.getNumFreeSpaces() != 1) {
						continue;
					}
				}

				auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

				if (freeBlockIt != chunk.freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_StagingAllocation = chunk.allocate(*freeBlockIt, allocInfo);
					buffer.m_StagingChunk = &chunk;

					//TODO write createInfo.data to stagingBuffer
					hasStaging = true;
					break;
				}

			}
			if (!hasStaging) {
				//if this is the second time we arives here the program can't continue from here
				if (i == 2) {
					LOGD("couldn't find staging memory for vulkanBuffer");
					ng::debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
				}
				VkResult result;
				auto it = addChunk(&m_StagingChunks, &result);
				if (result != VK_SUCCESS) {

					for (auto& chunk : m_StagingChunks) {
						if (chunk.getTotalFreeSpace() >= createInfo.size) {
							chunk.defragment(m_VulkanDevice);
						}
					}
					continue;
				}

				auto freeBlockIt = it->getClosestMatch(createInfo.size);

				if (freeBlockIt != it->freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
					buffer.m_StagingChunk = &*it;

					//TODO write createInfo.data to stagingBuffer
					hasStaging = true;
				}

				//should try to defrag chunks and then try again
				LOGD("failed to create Staging Buffer")
			}
		}

		//device-memory
		for (int i = 0; i < 2 && !hasStaging; ++i) {
			for (auto& chunk : m_DeviceChunks) {

				auto freeBlockIt = chunk.getClosestMatch(createInfo.size);

				if (freeBlockIt != chunk.freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_Allocation = chunk.allocate(*freeBlockIt, allocInfo);
					buffer.m_DeviceChunk = &chunk;

					//TODO write data in stagingBuffer to deviceMemory
					inDeviceMemory = true;
					//done
					return;
				}

			}
			if (!inDeviceMemory) {
				//if this is the second time we arives here the program can't continue from here
				if (i == 2) {
					LOGD("couldn't find staging memory for vulkanBuffer");
					ng::debug::exitFatal("couldn't find staging memory for vulkanBuffer", -1);
				}
				VkResult result;
				auto it = addChunk(&m_DeviceChunks, &result);
				if (result != VK_SUCCESS) {

					for (auto& chunk : m_StagingChunks) {
						if (chunk.getTotalFreeSpace() >= createInfo.size) {
							chunk.defragment(m_VulkanDevice);
						}
					}
					continue;
				}

				auto freeBlockIt = it->getClosestMatch(createInfo.size);

				if (freeBlockIt != it->freeBlocks.end()) {
					VulkanAllocationCreateInfo allocInfo;
					allocInfo.size = createInfo.size;
					allocInfo.dataSize = createInfo.dataSize;
					buffer.m_StagingAllocation = it->allocate(*freeBlockIt, allocInfo);
					buffer.m_StagingChunk = &*it;

					//TODO write createInfo.data to stagingBuffer
					hasStaging = true;
				}

				//should try to defrag chunks and then try again
				LOGD("failed to create Staging Buffer")
			}
		}

	}


public:

	VulkanBufferAllocator(ng::vulkan::VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkMemoryAlignment alignment) {
		m_VulkanDevice = vulkanDevice;
		m_MemoryFlags = flags;
		m_MemoryAlignment = alignment;
	}

	void createBuffer(VulkanBufferCreateInfo createInfo, VulkanBuffer& buffer) {
		
		if (m_MemoryFlags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			createDeviceLocalWithStaging(createInfo, buffer);
		}
		else if (m_MemoryFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
			createMappableDeviceLocal(createInfo, buffer);
		}


	}

};

