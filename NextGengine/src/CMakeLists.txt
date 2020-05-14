# CMakeList.txt : CMake project for NextGengine, include source and define
# project specific logic here.
#

#cmake_minimum_required (VERSION 3.8)

#dissable compiling unecessary glfw stuff
#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


#add_subdirectory("C:/C&C++/libs/glfw_source")

#find_package(glfw3 3.3 REQUIRED)
#find_package(Vulkan REQUIRED)

# Add source to this project's executable.
#add_executable (NextGengine "NextGengine.cpp" "NextGengine.h" "Vulkan/vulkan_base.cpp" "Vulkan/vulkan_instance.cpp")

# TODO: Add tests and install targets if needed.
#target_link_libraries(NextGengine glfw)
#target_link_libraries(NextGengine Vulkan::vulkan)