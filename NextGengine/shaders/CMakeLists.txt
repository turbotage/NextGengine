cmake_minimum_required(VERSION 3.17)

set(SHADER_WORKING_DIR "${CMAKE_SOURCE_DIR}/NextGengine/shaders/")

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
BUILD_SPIRV_SHADER("triangle/helloTriangle.frag" "triangle/helloTriangle.frag.spv")
BUILD_SPIRV_SHADER("triangle/helloTriangle.vert" "triangle/helloTriangle.vert.spv")




set(SHADER_SOURCES "triangle/helloTriangle.frag.spv"
					"triangle/helloTriangle.vert.spv")

add_custom_target(shaders ALL DEPENDS ${SHADER_SOURCES})
