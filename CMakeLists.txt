# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.17)

#set( CMAKE_VERBOSE_MAKEFILE on )

project ("NextGengine")

include_directories("NextGengine")

# Include sub-projects.
add_subdirectory ("NextGengine")
add_subdirectory ("SamplesAndTests")


target_compile_features(NextGengine PUBLIC cxx_std_17)
set_target_properties(NextGengine PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(NextGengineSample PUBLIC cxx_std_17)
set_target_properties(NextGengineSample PROPERTIES CXX_EXTENSIONS OFF)