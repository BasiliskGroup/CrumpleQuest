#version 330 core

struct textArray {
    sampler2DArray array;
};

struct MaterialData {
    vec3 color;

    uint albedoArray;
    uint albedoIndex;
    uint normalArray;
    uint normalIndex;

    float roughness;
    float subsurface;
    float sheen;
    float sheenTint;
    float anisotropic;
    float specular;
    float metallicness;
    float clearcoat;
    float clearcoatGloss;
};

in vec3 normal;
in vec2 uv;
flat in MaterialData material;

uniform sampler2D uTexture;
uniform textArray textureArrays[4];

out vec4 fragColor;

void main() {
    vec3 globalLight = normalize(vec3(.5, 1, .25));
    float brightness = (dot(normal, globalLight) + 1) / 2;

    fragColor = brightness * texture(textureArrays[material.albedoArray].array, vec3(uv, material.albedoIndex));;
} 