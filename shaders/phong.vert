#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal_matrix;  // transpose(inverse(mat3(model)))

out vec3 frag_pos;
out vec3 frag_normal;

void main() {
    vec4 world_pos = model * vec4(aPos, 1.0);
    frag_pos    = vec3(world_pos);
    frag_normal = normalize(normal_matrix * aNormal);
    gl_Position = projection * view * world_pos;
}
