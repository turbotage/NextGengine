#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    Vec4f gl_Position;
};

layout(location = 0) out Vec3f fragColor;

Vec2f positions[3] = Vec2f[](
    Vec2f(0.0, -0.5),
    Vec2f(0.5, 0.5),
    Vec2f(-0.5, 0.5)
);

Vec3f colors[3] = Vec3f[](
    Vec3f(1.0, 0.0, 0.0),
    Vec3f(0.0, 1.0, 0.0),
    Vec3f(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = Vec4f(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}