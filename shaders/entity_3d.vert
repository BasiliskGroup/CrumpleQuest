#version 330 core

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 uv;
out vec3 normal;

void main() {
    normal = vNormal;
    uv = vUV;

    gl_Position = uProjection * uView * uModel * vec4(vPosition, 1.0);
}
