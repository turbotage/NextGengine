#include "allocator.h"

void ng::memory::Allocator::init(uint64 size)
{	
	uint8 rest = size % MEMORY_ALIGNMENT;
	m_MemorySize = size - rest;
	m_FreeMemorySize = m_MemorySize;
	std::pair<uint64, uint64> freeSpace;
	freeSpace.second = 0;
	freeSpace.first = m_MemorySize;
	m_FreeSpaces.insert(freeSpace);
	printf("%" PRIu64 "\n", m_MemorySize);
}

ng::vma::Buffer ng::vma::VulkanMemoryAllocator::VkMemAllocator::createBuffer(VkDeviceSize size)
{
	VkDeviceSize allocSize = size + MEMORY_ALIGNMENT - (size % MEMORY_ALIGNMENT); //fix size to right mem-alignment

	auto ret = m_FreeSpaces.findSuitableFreeSpace(allocSize);
	Buffer buffer;
	buffer.buffer = &m_Buffer;
	buffer.offset = ret.first;
	buffer.size = allocSize;

	printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", buffer.offset, buffer.size); //show allocation parameters
	auto ret = m_Allocations.insert(std::pair<uint64, uint32>(buffer.offset, buffer.size)); // insert allocation in bin tree

	m_FreeSpaces.erase(ret.first, ret.second); //remove the old freespace
											   //insert the new freespace
	m_FreeSpaces.insert(buffer.offset + buffer.size, ret.second - buffer.size);
	m_FreeMemorySize -= allocSize;
	return buffer;
}

void ng::vma::VulkanMemoryAllocator::VkMemAllocator::freeBuffer(Buffer buffer)
{
	//function is implemented really ugly, not gonna bother changing it for now

	//print a VkDeviceSize printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters

	//TODO: change to use new FreeSpaces class

	auto currentAllocIt = m_Allocations.find(buffer.offset);
	if (currentAllocIt == m_Allocations.end()) {
		std::runtime_error("failed to find allocation to free\n");
	}
	//the new freeSpace that will be inserted
	std::pair<VkDeviceSize, VkDeviceSize> newFreeSpace;

	//if this isn't the first allocation there might be freespace infront of it or after it
	if (currentAllocIt != m_Allocations.begin()) {
		auto prevAllocIt = --currentAllocIt;
		//if there is a freespace before allocation
		if (prevAllocIt->first + prevAllocIt->second != currentAllocIt->first) {
			if (currentAllocIt != m_Allocations.end()) {
				//happens if allocations isn't last one
				auto nextAllocIt = ++currentAllocIt;
				//if there is a freespace after allocation
				if (currentAllocIt->first + currentAllocIt->second != nextAllocIt->first) {
					/* remove the freespaces to the right and left,
					and insert new one starting at rigth offset with size of all 3 blocks
					(right freespace, allocation, left freespace)*/
					auto prevFreeSpaceIt = std::find_if(m_FreeSpaces.begin(), m_FreeSpaces.end(), prevAllocIt->first + prevAllocIt->second);
					auto nextFreeSpaceIt = std::find_if(m_FreeSpaces.begin(), m_FreeSpaces.end(), currentAllocIt->first + currentAllocIt->second);
					newFreeSpace.second = prevFreeSpaceIt->second;
					newFreeSpace.first = prevFreeSpaceIt->first + currentAllocIt->second + nextFreeSpaceIt->first;
					m_FreeSpaces.erase(prevFreeSpaceIt);
					m_FreeSpaces.erase(nextFreeSpaceIt);
				}
				else {
					/*remove the freespace to the left and insert new one starting at left
					freespace offset with size (left freespace size) + (old allocation size)*/
					auto prevFreeSpaceIt = std::find_if(m_FreeSpaces.begin(), m_FreeSpaces.end(), prevAllocIt->first + prevAllocIt->second);
					newFreeSpace.second = prevFreeSpaceIt->second;
					newFreeSpace.first = prevFreeSpaceIt->first + currentAllocIt->second;
					m_FreeSpaces.erase(prevFreeSpaceIt);
				}
			}
			else {
				//happens if allocation is the last one
				auto prevFreeSpaceIt = std::find_if(m_FreeSpaces.begin(), m_FreeSpaces.end(), prevAllocIt->first + prevAllocIt->second);
				newFreeSpace.second = prevFreeSpaceIt->second;
				if (currentAllocIt->first + currentAllocIt->second != m_MemorySize) {
					//if last allocation isn't aligned with end of memory region
					newFreeSpace.first = m_MemorySize - newFreeSpace.second;
					auto rightFreeSpaceIt = std::find_if(m_FreeSpaces.begin(), m_FreeSpaces.end(), currentAllocIt->first + currentAllocIt->second);
					m_FreeSpaces.erase(rightFreeSpaceIt);
				}
				else {
					newFreeSpace.first = prevFreeSpaceIt->first + currentAllocIt->second;
				}
				m_FreeSpaces.erase(prevFreeSpaceIt);
			}
		}
		else {
			//if there is no freespace to the left or right of the allocation
			//make new freespace as big as old allocation
			newFreeSpace.first = currentAllocIt->second;
			newFreeSpace.second = currentAllocIt->first;
		}
	}
	else {
		//if the allocation is the first one, make new freespace
		newFreeSpace.second = 0;
		if (currentAllocIt != m_Allocations.end()) {
			auto nextAllocIt = ++currentAllocIt;
			if (currentAllocIt->first != 0) {
				m_FreeSpaces.erase(std::find_if());
			}
			if (currentAllocIt->first + currentAllocIt->second != nextAllocIt->first) {
				
			}
		}
		newFreeSpace.first = currentAllocIt->first + currentAllocIt->second;
		
	}

	m_Allocations.erase(currentAllocIt);
	m_FreeSpaces.insert(newFreeSpace);
}


auto ng::memory::Allocator::allocate(uint32 size)
{
	uint32 allocSize = size + MEMORY_ALIGNMENT - (size % MEMORY_ALIGNMENT); //fix size to rigth mem-alignment

	auto it = m_FreeSpaces.lower_bound(allocSize); //find the freeSpace with size nearest to wanted allocationSize
	if (it == m_FreeSpaces.end()) {
		std::runtime_error("failed to allocate\n");
	}
	Allocation alloc; //allocation to return
	alloc.offset = it->second;
	alloc.size = allocSize;

	printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", alloc.offset, alloc.size); //show allocation parameters
	auto ret = m_Allocations.insert(std::pair<uint64, uint32>(alloc.offset, alloc.size)); // insert allocation in bin tree

	std::pair<uint64, uint64> freeSpace; //the new freespace
	freeSpace.first = it->first - alloc.size;
	freeSpace.second = it->second + alloc.size;

	m_FreeSpaces.erase(it); //remove the old freespace
	m_FreeSpaces.insert(freeSpace); //insert the new freespace
	m_FreeMemorySize -= allocSize;
	return ret;
}

bool ng::memory::Allocator::free(Allocation allocation)
{
	auto it = m_Allocations.find(allocation.offset);
	if (it == m_Allocations.end()) {
		std::runtime_error("failed to allocate\n");
	}
	m_Allocations.erase(it);

	auto it2 = m_FreeSpaces.find(allocation.offset + allocation.size);
	if (it2 == m_FreeSpaces.end()) {
		std::pair<uint64, uint64> freeSpace;
		freeSpace.second = allocation.offset;
		freeSpace.first = allocation.size;
		printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters
		m_FreeSpaces.insert(freeSpace);
		return false;
	}
	std::pair<uint64, uint64> freeSpace;
	freeSpace.second = allocation.offset;
	freeSpace.first = allocation.size + it->first;
	printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters
	m_FreeSpaces.erase(it2);
	m_FreeSpaces.insert(freeSpace);
	return true;
}

uint64 ng::memory::Allocator::getPossibleDefragSize()
{
	uint64 possDefragSize = 0;
	for (auto& freeSapce : m_FreeSpaces) {
		possDefragSize = freeSapce.first;
	}
}

uint32 ng::memory::Allocator::getDefragSpaceNum()
{
	return m_FreeSpaces.size() - 1;
}

ng::memory::DefragType ng::memory::Allocator::defragment()
{
	Allocation alloc;
	alloc.offset = 0;
	alloc.size = 0;
	if (getDefragSpaceNum() ) {
		DefragType returner;
		returner.offset = 0;
		return returner;
	}
	

}

void ng::vma::VulkanMemoryAllocator::createVkBufferAndMemory(
	VkBuffer* buffer, 
	VkDeviceMemory* memory, 
	VkDeviceSize size, 
	VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags properties, 
	VkSharingMode sharingMode)
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = sharingMode;

	if (vkCreateBuffer(*m_Device, &createInfo, nullptr, buffer) != VK_SUCCESS) {

	}
}
