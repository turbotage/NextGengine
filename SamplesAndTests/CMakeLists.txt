cmake_minimum_required (VERSION 3.17)



#Hello Triangle
add_executable(HelloTriangle "HelloTriangle/hello_triangle.cpp")
target_link_libraries(HelloTriangle NextGengine)
target_compile_features(HelloTriangle PUBLIC cxx_std_17)
set_target_properties(HelloTriangle PROPERTIES CXX_EXTENSIONS OFF)


#Push Constants
add_executable(PushConstants "PushConstants/push_constants.cpp")
target_link_libraries(PushConstants NextGengine)
target_compile_features(PushConstants PUBLIC cxx_std_17)
set_target_properties(PushConstants PROPERTIES CXX_EXTENSIONS OFF)