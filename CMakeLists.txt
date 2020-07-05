# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.17)

#set( CMAKE_VERBOSE_MAKEFILE on )

project ("NextGengine")

#For windows we wan't to export all symbols
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories("NextGengineCore")

# Include sub-projects.
add_subdirectory ("NextGengineCore")
add_subdirectory ("SamplesAndTests")
add_subdirectory ("Assets")

