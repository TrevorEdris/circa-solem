#version 410 core

layout(location = 0) in vec3  aPos;
layout(location = 1) in float aBrightness;
layout(location = 2) in float aPointSize;

uniform mat4 view;
uniform mat4 projection;

out float brightness;

void main() {
    brightness = aBrightness;

    vec4 view_pos = view * vec4(aPos, 1.0);
    float dist    = length(view_pos.xyz);

    // Size attenuation: larger when close, smaller when far
    gl_PointSize  = aPointSize / (dist * 0.3 + 1.0);

    gl_Position = projection * view_pos;
}
