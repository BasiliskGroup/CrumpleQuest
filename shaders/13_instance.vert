#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;

layout (location = 3) in vec3 instance_position;

uniform mat4 uProjection;
uniform mat4 uView;

out vec2 uv;
out vec3 normal;

void main() {
    uv = in_uv;
    normal = in_normal;
    gl_Position = uProjection * uView * vec4(in_position + instance_position, 1.0);
}