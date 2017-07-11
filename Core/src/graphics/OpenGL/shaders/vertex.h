#pragma once

const char* vertex0 = R"(
#version 330 core

layout(location = 0) in vec3 vertexPos_modelspace;

uniform mat4 MVP;

void main() {
	gl_Position = MVP * vec4(vertexPos_modelspace,1);
}
)";