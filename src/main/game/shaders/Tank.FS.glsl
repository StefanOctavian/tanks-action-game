#version 330
#define PI 3.1415926535897932384626433832795

in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_color;

layout(location = 0) out vec4 out_color;

uniform int HP;


void main()
{
    vec3 tangent = vec3(frag_normal.z, 0.0, -frag_normal.x);
    tangent = (tangent == vec3(0.0)) ? vec3(0.0, 0.0, -frag_position.y) : tangent;
    tangent = normalize(tangent);
    vec3 bitangent = cross(frag_normal, tangent);

    float tangent_component = dot(frag_position, tangent);
    float bitangent_component = dot(frag_position, bitangent);

    vec3 brown = vec3(0.3, 0.17, 0.0);
    vec3 dark_brown = vec3(0.17, 0.11, 0.0);
    vec3 color = frag_color;
    if (HP <= 2) {
        float percent = (sin(2 * PI * tangent_component) + cos(3 * PI * bitangent_component)) / 2.0;
        percent = smoothstep(-1.0, 1.0, percent);
        color = mix(brown, frag_color, percent);
    }
    if (HP == 1) {
        float perent = (sin(PI * bitangent_component) + cos(PI * tangent_component)) / 2.0;
        color = mix(dark_brown, color, perent);
    }
    out_color = vec4(color, 1);
}
