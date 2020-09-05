cmake_minimum_required (VERSION 3.17)

project("NextGengineTesting")

find_library(LibraryCache NAMES NextGengineCore REQUIRED)



#TestNG
add_executable(TestNG "TestNg/test_ng.cpp")
target_link_libraries(TestNG NextGengineCore)
target_compile_features(TestNG PUBLIC cxx_std_17)
set_target_properties(TestNG PROPERTIES CXX_EXTENSIONS OFF)


#TestResources
add_executable(TestResources "TestResources/test_resources.cpp")
target_link_libraries(TestResources NextGengineCore)
target_compile_features(TestResources PUBLIC cxx_std_17)
set_target_properties(TestResources PROPERTIES CXX_EXTENSIONS OFF)


#TestResources2
add_executable(TestResources2 "TestResources2/test_resources2.cpp")
target_link_libraries(TestResources2 NextGengineCore)
target_compile_features(TestResources2 PUBLIC cxx_std_17)
set_target_properties(TestResources2 PROPERTIES CXX_EXTENSIONS OFF)


#Hello Triangle
add_executable(HelloTriangle "HelloTriangle/hello_triangle.cpp")
target_link_libraries(HelloTriangle NextGengineCore)
target_compile_features(HelloTriangle PUBLIC cxx_std_17)
set_target_properties(HelloTriangle PROPERTIES CXX_EXTENSIONS OFF)


#Push Constants
add_executable(PushConstants "PushConstants/push_constants.cpp")
target_link_libraries(PushConstants NextGengineCore)
target_compile_features(PushConstants PUBLIC cxx_std_17)
set_target_properties(PushConstants PROPERTIES CXX_EXTENSIONS OFF)
