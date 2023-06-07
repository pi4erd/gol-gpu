#version 460 core

layout (location = 0) out vec4 fragColor;

layout (location = 0) in vec2 uv;

layout (binding = 0) uniform sampler2D mainTexture;

void main() {
    fragColor = texture(mainTexture, uv);
}