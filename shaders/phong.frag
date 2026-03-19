#version 410 core

in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 light_dir;     // normalized direction toward the light (world space)
uniform vec3 light_color;
uniform vec3 object_color;
uniform vec3 view_pos;

out vec4 frag_color;

void main() {
    vec3 N = normalize(frag_normal);
    vec3 L = normalize(light_dir);
    vec3 V = normalize(view_pos - frag_pos);
    vec3 R = reflect(-L, N);

    vec3 ambient  = 0.05 * object_color;
    vec3 diffuse  = max(dot(N, L), 0.0) * light_color * object_color;
    vec3 specular = pow(max(dot(R, V), 0.0), 32.0) * 0.4 * light_color;

    frag_color = vec4(ambient + diffuse + specular, 1.0);
}
