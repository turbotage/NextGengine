cmake_minimum_required (VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(VULKAN_SOURCES "Vulkan/vulkan_allocator.cpp" 
					"Vulkan/vulkan_context.cpp" 
					"Vulkan/vulkan_debug.cpp" 
					"Vulkan/vulkan_device.cpp" 
					"Vulkan/vulkan_instance.cpp" 
					"Vulkan/vulkan_pipeline.cpp" 
					"Vulkan/vulkan_renderpass.cpp"
					"Vulkan/vulkan_storage.cpp"
					"Vulkan/vulkan_swapchain.cpp"
					"Vulkan/vulkan_utility.cpp"
					"Vulkan/vulkan_window.cpp")

set(MEMORY_SOURCES "Memory/abstract_allocators.cpp"
					"Memory/abstract_allocators.h")

target_sources(NextGengine PRIVATE ${VULKAN_SOURCES} ${MEMORY_SOURCES})