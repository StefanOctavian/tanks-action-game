#version 330
// fragment shader for phong lighting with a single directional white light source

in vec3 frag_pos;
in vec3 frag_normal;
in vec3 frag_color;

out vec4 out_color;

uniform float ambient_light;
uniform vec3 light_dir;
uniform vec3 eye_position;

uniform float k_emmissive;
uniform float k_ambient;
uniform float k_diffuse;
uniform float k_specular;
uniform float k_shininess;

void main()
{
    vec3 normal = normalize(frag_normal);
    vec3 light_d = normalize(light_dir);
    vec3 eye_dir = normalize(eye_position - frag_pos);

    float ambient = ambient_light * k_ambient;
    float diffuse = max(dot(normal, light_d), 0.0) * k_diffuse;
    vec3 median_dir = normalize(light_d + eye_dir);
    float specular = 0;
    if (diffuse > 0)
        specular = pow(max(dot(median_dir, normal), 0.0), k_shininess) * k_specular;

    out_color = vec4(frag_color * (ambient + diffuse + specular), 1.0);
}