#pragma once

#include "../../def.h"

/**
	There is a memoryMutex for each vulkan_device, every thread doing work accessing or referencing 
	vulkan_memory from that vulkan_device, aka using a vulkan_buffer created from a vulkan_device
	should lock around that vulkan_device's mutex. 
	It is also expected that the lock shouldn't be held for to long since this 
	makes defragmentation and more importantly buffer creation in other threads inneficient and slow.

	Using memory example:

	void render() {
		std::unique_lock<std::mutex> lock(m_VulkanDevice->memoryMutex);
		render();
	}

	To think about. Should memoryMutex be recursive?


**/

namespace ng {
	namespace vulkan {

		namespace globals {
			//std::mutex g_MemoryLock;
		
		
		}

	}
}