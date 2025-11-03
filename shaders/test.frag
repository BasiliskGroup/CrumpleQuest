#version 330 core

in vec2 uv;

struct textArray {
    sampler2DArray array;
};

uniform textArray textureArrays[4];

out vec4 fragColor;

void main() {
    fragColor = texture(textureArrays[1].array, vec3(uv, 0));
} 