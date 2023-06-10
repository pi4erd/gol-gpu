#version 460 core
layout (location = 0) in vec2 TexCoords;
layout (location = 1) out vec4 color;

layout (location = 2) uniform vec3 textColor;
layout (binding = 0) uniform sampler2D text;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    //color = vec4(textColor, 1.0) * sampled;
    color = vec4(textColor, 1.0);
}
