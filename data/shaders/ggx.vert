#version 410 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 transform;

void main() {
    Position = aPosition;
    Normal = aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection_matrix * view_matrix * transform * vec4(aPosition, 1.0);
}
