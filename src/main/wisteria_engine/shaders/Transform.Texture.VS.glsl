#version 330
// A vertex shader allowing you to do an arbitrary linear transformation to the uv coordinates
// of the entire mesh. This lets you do things like scaling the texture with the mesh.

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

uniform mat4 WIST_MODEL_MATRIX;
uniform mat4 WIST_VIEW_MATRIX;
uniform mat4 WIST_PROJECTION_MATRIX;

uniform mat3 UV_TRANSFORM;

out vec3 frag_normal;
out vec3 frag_color;
out vec2 frag_tex_coord;

void main()
{
    // We use a change of basis matrix to transform from xyz to normal, over, cross:
    // we don't know the up or right vectors, but we don't need them. We only need two
    // vectors that are perpendicular to the normal, and each other. I will call them
    // over and cross, because one is over the plane of the face, and the other is the
    // cross product of the normal and over.
    vec3 v_over = vec3(-v_normal.z, 0.0, v_normal.x);
    v_over = v_over == vec3(0.0) ? vec3(0.0, 0.0, -v_normal.y) : v_over;
    v_over = normalize(v_over);
    vec3 v_cross = cross(v_normal, v_over);
    // check if over and cross are in the right order
    float triple_product = dot(v_normal, cross(v_over, v_cross));
    if (triple_product < 0.0) {
        vec3 aux = v_over;
        v_over = v_cross;
        v_cross = aux;
    }
    mat3 change_of_basis = inverse(mat3(v_normal, v_over, v_cross));
    // we are only interested in the coordinates inside the face, not the normal one
    vec2 face_coords = (change_of_basis * v_position).yz;

    // Compute the same normal, over and cross coordinates for the transformed position.
    vec3 t_pos = UV_TRANSFORM * v_position;
    vec3 t_normal = normalize(UV_TRANSFORM * v_normal);
    // vec3 t_over = normalize(vec3(t_normal.y, -t_normal.x, 0.0));
    vec3 t_over = normalize(UV_TRANSFORM * v_over);
    // vec3 t_cross = cross(t_normal, t_over);
    vec3 t_cross = normalize(UV_TRANSFORM * v_cross);
    mat3 t_change_of_basis = inverse(mat3(t_normal, t_over, t_cross));
    vec2 t_face_coords = (t_change_of_basis * t_pos).yz;

    // scale the uv coordinates proportionally to the change in the face coordinates
    vec2 uv_scale = t_face_coords / face_coords;
    frag_tex_coord = uv_scale * v_texture_coord;
    
    mat4 MVP = WIST_PROJECTION_MATRIX * WIST_VIEW_MATRIX * WIST_MODEL_MATRIX;
    gl_Position = MVP * vec4(v_position, 1.0);
    frag_normal = normalize(mat3(WIST_MODEL_MATRIX) * v_normal);
    frag_color = v_color;
}
