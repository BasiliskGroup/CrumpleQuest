#version 330 core

in vec3 normal;
in vec2 uv;

uniform sampler2D uTexture;

out vec4 fragColor;

void main() {
    vec3 globalLight = normalize(vec3(.5, .25, 1));
    float brightness = (dot(normal, globalLight) + 1) / 2;

    vec4 textureValue = texture(uTexture, uv);

    fragColor = vec4((0.75 + brightness / 4) * textureValue.rgb, textureValue.a);
} 