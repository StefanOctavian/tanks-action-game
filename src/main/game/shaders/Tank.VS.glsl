#version 330
#define PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 3) in vec3 v_color;

uniform mat4 WIST_MODEL_MATRIX;
uniform mat4 WIST_VIEW_MATRIX;
uniform mat4 WIST_PROJECTION_MATRIX;
uniform int HP;
uniform float SUB_HUE;
uniform float ADD_HUE;

out vec3 frag_position;
out vec3 frag_normal;
out vec3 frag_color;

// conversion function from https://stackoverflow.com/questions/68901847/opengl-esconvert-rgb-to-hsv-not-hsl
vec3 HSVtoRGB(in vec3 HSV)
{
    float H   = HSV.x;
    float R   = abs(H * 6.0 - 3.0) - 1.0;
    float G   = 2.0 - abs(H * 6.0 - 2.0);
    float B   = 2.0 - abs(H * 6.0 - 4.0);
    vec3  RGB = clamp( vec3(R,G,B), 0.0, 1.0 );
    return ((RGB - 1.0) * HSV.y + 1.0) * HSV.z;
}

// conversion after https://www.rapidtables.com/convert/color/rgb-to-hsv.html
vec3 RGBtoHSV(in vec3 RGB)
{
    float Cmax = max(RGB.r, max(RGB.g, RGB.b));
    float Cmin = min(RGB.r, min(RGB.g, RGB.b));
    float delta = Cmax - Cmin;
    vec3 HSV = vec3(0.0, 0.0, Cmax);
    HSV.x = (delta == 0.0)  ? 0.0 :
            (Cmax == RGB.r) ? mod((RGB.g - RGB.b) / delta, 6.0) :
            (Cmax == RGB.g) ? ((RGB.b - RGB.r) / delta + 2.0) :
            ((RGB.r - RGB.g) / delta + 4.0);
    HSV.x = HSV.x / 6.0;
    HSV.y = (Cmax == 0.0) ? 0.0 : delta / Cmax;
    return HSV;
}

vec3 apply_deformation(vec3 position, int degree)
{
    float dent_factor = 0.1 * degree;
    //float s = mod(position.z, 0.35f) > 0.175f ? 1.0f : -1.0f;
    float direction = degree < 2 ? position.z : position.x * position.y * 0.9;
    float dent = dent_factor * (1.0 + sin(direction * 3 * PI));
    return position * (1.0 + sin(direction * 3 * PI) * dent_factor);
    //return position + vec3(0, 0, s * dent) + vec3(-s * dent * direction, 0, 0);
}

void main()
{
    vec3 v_affected_position = apply_deformation(v_position, 3 - HP);
    frag_position = v_affected_position;
    frag_normal = v_normal;
    frag_color = HSVtoRGB(RGBtoHSV(v_color) + vec3(ADD_HUE - SUB_HUE, 0, 0));
    mat4 MVP = WIST_PROJECTION_MATRIX * WIST_VIEW_MATRIX * WIST_MODEL_MATRIX;
    gl_Position = MVP * vec4(v_affected_position, 1.0);
}
