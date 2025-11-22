#version 330 core

in vec2 uv;

uniform sampler2D uTexture;

out vec4 fragColor;

void main() {
    fragColor = texture(uTexture, uv);
} 