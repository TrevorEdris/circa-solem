#version 410 core

layout(location = 0) in vec2 aOffset;  // corner offsets: (+-0.5, +-0.5)

uniform mat4  view;
uniform mat4  projection;
uniform vec3  uCenter;  // world-space center (Sun position)
uniform float uSize;    // billboard half-size in AU

out vec2 vUV;

void main() {
    // Extract camera right and up from view matrix columns.
    vec3 right = normalize(vec3(view[0][0], view[1][0], view[2][0]));
    vec3 up    = normalize(vec3(view[0][1], view[1][1], view[2][1]));

    vec3 world  = uCenter + (right * aOffset.x + up * aOffset.y) * uSize;
    vUV         = aOffset + vec2(0.5);  // remap to [0,1]
    gl_Position = projection * view * vec4(world, 1.0);
}
