#version 410 core

layout (location = 0) in vec3 aPosition;

uniform mat4 transform;
uniform mat4 light_transform;

void main() {
    gl_Position = light_transform * transform * vec4(aPosition, 1.0f);
}
