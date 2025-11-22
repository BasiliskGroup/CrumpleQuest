#version 330 core

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vUV;

out vec2 uv;

void main() {
    uv = vUV;
    gl_Position = vec4(vPosition, 0.0, 1.0);
}
