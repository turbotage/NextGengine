#include "vkbuffer_region_allocator.h"

//VkBuffer

ng::vma::Buffer::Buffer()
	: m_BufferCopies(0)
{

}

ng::vma::Buffer::Buffer(VkDeviceSize offset, VkDeviceSize size, VkBuffer * buffer)
	: m_Offset(offset), m_Size(size), m_Buffer(buffer)
{
	m_Data = malloc(size);
}

ng::vma::Buffer::Buffer(Buffer & buf)
{
	buf.m_BufferCopies++;
	m_BufferCopies = buf.m_BufferCopies;
	m_Buffer = buf.m_Buffer;
	m_Data = buf.m_Buffer;
	m_Offset = buf.m_Offset;
	m_Size = buf.m_Size;
}







//VkBufferRegionAllocator

ng::vma::VkBufferRegionAllocator::VkBufferRegionAllocator(VkDeviceSize memorySize, VkMemoryAlignment memoryAlignment)
	: m_MemorySize(memorySize), m_MemoryAlignment(memoryAlignment)
{

}

std::pair<VkDeviceSize, VkDeviceSize> ng::vma::VkBufferRegionAllocator::findSuitableFreeSpace(VkDeviceSize size)
{
	return m_FreeSpaces.findSuitableFreeSpace(size);
}

bool ng::vma::VkBufferRegionAllocator::isInBufferRegion(Buffer * buffer)
{
	auto alloc = m_Allocations.find(buffer->m_Offset);
	if (alloc != m_Allocations.end()) {
		if (alloc->second.buffer == buffer) {
			return true;
		}
	}
	return false;
}

ng::vma::Buffer* ng::vma::VkBufferRegionAllocator::createBuffer(VkDeviceSize size)
{

	VkDeviceSize allocSize = size + m_MemoryAlignment -(size % m_MemoryAlignment); //fix size to right mem-alignment

	auto freeSpaceRet = m_FreeSpaces.findSuitableFreeSpace(allocSize);
	Buffer* retBuffer = new Buffer(freeSpaceRet.first, freeSpaceRet.second, &buffer);

	//printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", buffer.offset, buffer.size); //show allocation parameters
	m_Allocations.insert(freeSpaceRet.first, freeSpaceRet.second, retBuffer);

	//buffer.setOffsetAndSizeIterator(allocationRet);

	m_FreeSpaces.erase(freeSpaceRet.first, freeSpaceRet.second); //remove the old freespace
																 //insert the new freespace
	m_FreeSpaces.insert(retBuffer->m_Offset + retBuffer->m_Size, freeSpaceRet.second - retBuffer->m_Size);
	m_FreeMemorySize -= allocSize;
	return retBuffer;
}

ng::vma::Buffer* ng::vma::VkBufferRegionAllocator::createBuffer(VkDeviceSize offset, VkDeviceSize size)
{
	Buffer* retBuffer = new Buffer(offset, size, &buffer);

	m_Allocations.insert(offset, size, retBuffer);

	m_FreeSpaces.erase(offset, size);
	m_FreeSpaces.insert(retBuffer->m_Offset + retBuffer->m_Size, size - retBuffer->m_Size);
	return retBuffer;
}

void ng::vma::VkBufferRegionAllocator::freeBuffer(Buffer* buffer)
{

	//print a VkDeviceSize printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters

	if (buffer->m_BufferCopies != 0) {
		buffer->m_BufferCopies--;
		return;
	}

	auto currentAllocIt = m_Allocations.find(buffer->m_Offset);
	if (currentAllocIt == m_Allocations.end()) {
		std::runtime_error("failed to find allocation to free\n");
	}

	//if this is neither the first nor the last allocation
	if ((currentAllocIt != m_Allocations.begin()) && (++currentAllocIt != m_Allocations.end())) {
		auto prevAllocIt = --currentAllocIt;
		auto nextAllocIt = ++currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second.size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second.size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second.size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.size != nextAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.size);
			VkDeviceSize newFreeSpaceOffset = currentAllocIt->first;
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second.size);
		}
	}
	//if this is the only allocation there is
	else if ((currentAllocIt == m_Allocations.begin()) && (++currentAllocIt == m_Allocations.end())) {
		m_FreeSpaces.clear();
		m_FreeSpaces.insert(0, m_MemorySize);
	}
	//if this is the first allocation
	else if (currentAllocIt == m_Allocations.begin()) {
		auto nextAllocIt = ++currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((currentAllocIt->first != 0) && (currentAllocIt->first + currentAllocIt->second.size != nextAllocIt->first)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.size);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.size + nextFreeSpace.second;
			m_FreeSpaces.erase(0, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (currentAllocIt->first != 0) {
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(0);
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.size;
			m_FreeSpaces.erase(0, prevFreeSpace.second);

			m_FreeSpaces.insert(0, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.size != nextAllocIt->first){
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(0, currentAllocIt->second.size);
		}
	}
	//if this is the last allocation
	else {
		auto prevAllocIt = --currentAllocIt;
		//if there is freespace to the right and left of the allocation
		if ((prevAllocIt->first + prevAllocIt->second.size != currentAllocIt->first) && (currentAllocIt->first + currentAllocIt->second.size != m_MemorySize)) {
			/* remove the freespaces to the right and left,
			and insert new one starting at rigth offset with size of all 3 blocks
			(right freespace, allocation, left freespace)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.size);
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.size + nextFreeSpace.second;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the left
		else if (prevAllocIt->first + prevAllocIt->second.size != currentAllocIt->first) {
			/*remove the freespace to the left and insert new one starting at left
			freespace offset with size (left freespace size) + (old allocation size)*/
			auto prevFreeSpace = m_FreeSpaces.findSpaceWithOffset(prevAllocIt->first + prevAllocIt->second.size);
			VkDeviceSize newFreeSpaceOffset = prevFreeSpace.first;
			VkDeviceSize newFreeSpaceSize = prevFreeSpace.second + currentAllocIt->second.size;
			m_FreeSpaces.erase(prevFreeSpace.first, prevFreeSpace.second);

			m_FreeSpaces.insert(newFreeSpaceOffset, newFreeSpaceSize);
		}
		//if there is freespace to the right
		else if (currentAllocIt->first + currentAllocIt->second.size != m_MemorySize) {
			auto nextFreeSpace = m_FreeSpaces.findSpaceWithOffset(currentAllocIt->first + currentAllocIt->second.size);
			VkDeviceSize newFreeSpaceSize = currentAllocIt->second.size + nextFreeSpace.second;
			m_FreeSpaces.erase(nextFreeSpace.first, nextFreeSpace.second);

			m_FreeSpaces.insert(currentAllocIt->first, newFreeSpaceSize);
		}
		//if allocation isn't surrounded by any freespaces
		else {
			m_FreeSpaces.insert(currentAllocIt->first, currentAllocIt->second.size);
		}
	}
	
	m_Allocations.erase(currentAllocIt);
}

void ng::vma::VkBufferRegionAllocator::defragment(VkBuffer stagingBuffer)
{
	
}

