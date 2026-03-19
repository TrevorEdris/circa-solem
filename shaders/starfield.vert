#version 410 core

layout(location = 0) in vec3  aPos;
layout(location = 1) in float aBrightness;
layout(location = 2) in float aPointSize;

uniform mat4 view;
uniform mat4 projection;

out float brightness;

void main() {
    brightness  = aBrightness;
    gl_PointSize = aPointSize;

    vec4 clip_pos = projection * view * vec4(aPos, 1.0);

    // Force depth to 1.0 (far plane) so stars render behind all geometry
    // regardless of world-space radius or scale system.
    // Standard skybox technique: gl_Position.z = gl_Position.w
    gl_Position = clip_pos.xyww;
}
