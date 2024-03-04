#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

uniform mat4 WIST_MODEL_MATRIX;
uniform mat4 WIST_VIEW_MATRIX;
uniform mat4 WIST_PROJECTION_MATRIX;

out vec3 frag_normal;
out vec3 frag_color;
out vec2 frag_tex_coord;

void main()
{
    frag_normal = mat3(WIST_MODEL_MATRIX) * v_normal;
    frag_color = v_color;
    frag_tex_coord = v_texture_coord;
    mat4 MVP = WIST_PROJECTION_MATRIX * WIST_VIEW_MATRIX * WIST_MODEL_MATRIX;
    gl_Position = MVP * vec4(v_position, 1.0);
}
