# CMakeList.txt : CMake project for NextGengine, include source and define
# project specific logic here.


cmake_minimum_required (VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source to this project's executable.
set(SOURCES "NextGengine.h")

add_library(NextGengine ${SOURCES})
add_subdirectory("src")

# Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan Lib = ${Vulkan_LIBRARY}")
target_include_directories(NextGengine PRIVATE Vulkan::Vulkan)
target_link_libraries(NextGengine Vulkan::Vulkan)

# GLFW
#dissable compiling unecessary glfw stuff
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("lib/glfw")
target_link_libraries(NextGengine glfw)

# GLM
#dissable compiling unecessary glm stuff
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)

add_subdirectory("lib/glm")
target_link_libraries(NextGengine glm)

# GLI
#dissable compiling unecessary glm stuff
add_subdirectory("lib/gli")
target_link_libraries(NextGengine gli)

