#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in Vec3f fragColor;

layout(location = 0) out Vec4f outColor;

void main() {
    outColor = Vec4f(fragColor, 1.0);
}