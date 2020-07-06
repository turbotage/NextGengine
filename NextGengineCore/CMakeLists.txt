# CMakeList.txt : CMake project for NextGengine, include source and define
# project specific logic here.

cmake_minimum_required (VERSION 3.17)

project("NextGengineCore")

set(BUILD_SHARED_LIBS ON)

# Add source to this project's executable.
set(SOURCES "NextGengineCore.h" "pch.h")

add_library(NextGengineCore SHARED ${SOURCES})
add_subdirectory("src")

#target_precompile_headers(NextGengineCore PUBLIC "pch.h")

set(BUILD_TESTING OFF)

# Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan FOUND = ${Vulkan_FOUND}")
message(STATUS "Vulkan Include = ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan Lib = ${Vulkan_LIBRARY}")
target_include_directories(NextGengineCore PUBLIC Vulkan::Vulkan)
target_link_libraries(NextGengineCore PUBLIC Vulkan::Vulkan)

# GLFW
#dissable compiling unecessary glfw stuff
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/glfw")
target_link_libraries(NextGengineCore PUBLIC glfw)

# GLM
#dissable compiling unecessary glm stuff
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/glm")
target_link_libraries(NextGengineCore PUBLIC glm)

# GLI
#dissable compiling unecessary gli stuff
#add_subdirectory("lib/gli")
#target_link_libraries(NextGengineCore PUBLIC gli)

# KTX-Software
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_DOC OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_LOADTEST_APPS OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/KTX-Software")
target_link_libraries(NextGengineCore PUBLIC ktx)



# SET C++ STANDARD
target_compile_features(NextGengineCore PUBLIC cxx_std_17)
set_target_properties(NextGengineCore PROPERTIES CXX_EXTENSIONS OFF)

