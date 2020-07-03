cmake_minimum_required(VERSION 3.17)

set(SHADER_WORKING_DIR "${CMAKE_SOURCE_DIR}/Assets/shaders/")

function(BUILD_SPIRV_SHADER INPUT_FILE OUTPUT_FILE)
	add_custom_command(
		OUTPUT ${OUTPUT_FILE}
		COMMAND glslc ${INPUT_FILE} -o ${OUTPUT_FILE} # REQUIRES GLSLC in path
		MAIN_DEPENDENCY ${INPUT_FILE}
		WORKING_DIRECTORY ${SHADER_WORKING_DIR}
	)
endfunction()


message("Shader-Working-Dir: ${SHADER_WORKING_DIR}")


#triangle shaders
BUILD_SPIRV_SHADER("HelloTriangle/helloTriangle.frag" "HelloTriangle/helloTriangle.frag.spv")
BUILD_SPIRV_SHADER("HelloTriangle/helloTriangle.vert" "HelloTriangle/helloTriangle.vert.spv")
#push constants shaders
BUILD_SPIRV_SHADER("PushConstants/pushConstants.frag" "PushConstants/pushConstants.frag.spv")
BUILD_SPIRV_SHADER("PushConstants/pushConstants.vert" "PushConstants/pushConstants.vert.spv")
#test resources shaders
BUILD_SPIRV_SHADER("TestResources/testResources.frag" "TestResources/testResources.frag.spv")
BUILD_SPIRV_SHADER("TestResources/testResources.vert" "TestResources/testResources.vert.spv")


set(SHADER_SOURCES "HelloTriangle/helloTriangle.frag.spv"
					"HelloTriangle/helloTriangle.vert.spv"
					"PushConstants/pushConstants.frag.spv"
					"PushConstants/pushConstants.vert.spv"
					"TestResource/testResource.frag.spv"
					"TestResource/testResource.vert.spv")

add_custom_target(shaders ALL DEPENDS ${SHADER_SOURCES})
