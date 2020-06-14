cmake_minimum_required (VERSION 3.17)


add_executable(NextGengineSample "main.cpp")
target_link_libraries(NextGengineSample NextGengine)
target_compile_features(NextGengineSample PUBLIC cxx_std_17)
set_target_properties(NextGengineSample PROPERTIES CXX_EXTENSIONS OFF)