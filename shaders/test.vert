#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;

out vec2 uv;

void main() {
    uv = vUV;
    gl_Position =  vec4(vPosition, 1.0);
}
