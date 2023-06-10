#version 460 core
layout (location = 0) in vec4 vertex;
layout (location = 0) out vec2 TexCoords;

layout (location = 1) uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
