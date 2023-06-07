#version 460 core

layout (location = 0) out vec4 fragColor;

layout (location = 0) in vec2 uv;

layout (binding = 0) uniform sampler2D mainTexture;

layout (location = 2) uniform float zoom = 1;
layout (location = 3) uniform float aspectRatio = 1;
layout (location = 4) uniform float posx = 0;
layout (location = 5) uniform float posy = 0;

void main() {
    vec2 localuv = uv;
    localuv = (localuv * 2 - 1) * vec2(aspectRatio, 1) / zoom + vec2(posx, posy);

    fragColor = texture(mainTexture, localuv);
}