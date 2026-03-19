#version 410 core

layout(location = 0) in vec3  aPos;
layout(location = 1) in float aAlpha;

out float vAlpha;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vAlpha      = aAlpha;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
