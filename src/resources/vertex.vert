#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vUv;

layout (location = 0) out vec2 uv;

void main() {
    gl_Position = vec4(vPos, 1);
    uv = vUv;
}