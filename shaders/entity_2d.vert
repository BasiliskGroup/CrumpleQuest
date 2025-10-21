#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;


uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 uv;

void main() {
    uv = vUV;
    gl_Position = uProjection * uView * uModel * vec4(vPosition, 1.0);
}
