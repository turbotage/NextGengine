cmake_minimum_required (VERSION 3.17)


set(VULKAN_SOURCES "Vulkan/vulkan_allocator.cpp" 
					"Vulkan/vulkan_context.cpp" 
					"Vulkan/vulkan_debug.cpp"
					"Vulkan/vulkan_descriptor.cpp"
					"Vulkan/vulkan_device.cpp" 
					"Vulkan/vulkan_instance.cpp" 
					"Vulkan/vulkan_pipeline.cpp" 
					"Vulkan/vulkan_renderpass.cpp"
					"Vulkan/vulkan_storage.cpp"
					"Vulkan/vulkan_swapchain.cpp"
					"Vulkan/vulkan_thread.cpp"
					"Vulkan/vulkan_utility.cpp"
					"Vulkan/vulkan_window.cpp")

set(MEMORY_SOURCES "Memory/abstract_allocators.cpp"
					"Memory/abstract_allocators.h")

set(THREADING_SOURCES "Threading/thread_pool.cpp"
						"Threading/thread_safe_queue.cpp")

set(LOCAL_SOURCES "ng_utility.cpp")

target_sources(NextGengine PRIVATE ${VULKAN_SOURCES} ${MEMORY_SOURCES} ${THREADING_SOURCES} ${LOCAL_SOURCES})