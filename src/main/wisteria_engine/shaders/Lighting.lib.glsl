uniform float WIST_SCENE_AMBIENT;
uniform vec4 WIST_LIGHTS[8];
uniform vec4 WIST_EYE_POSITION;

uniform float WIST_MATERIAL_AMBIENT;
uniform float WIST_MATERIAL_DIFFUSE;
uniform float WIST_MATERIAL_SPECULAR;
uniform float WIST_MATERIAL_SHININESS;

vec3 lightDirection(vec4 light, vec3 frag_pos)
{
    return light.w == 0.0 ? normalize(light.xyz) : normalize(light.xyz - frag_pos);
}

vec3 eyeDirection(vec3 frag_pos)
{
    return WIST_EYE_POSITION.w == 0.0 ? normalize(WIST_EYE_POSITION.xyz) 
                                      : normalize(WIST_EYE_POSITION.xyz - frag_pos);
}

vec3 wist_phongLighting(vec4 light, vec3 frag_pos, vec3 frag_normal, vec3 frag_color)
{
    vec3 normal = normalize(frag_normal);
    vec3 light_dir = lightDirection(light, frag_pos);
    vec3 eye_dir = eyeDirection(frag_pos);

    float ambient = WIST_SCENE_AMBIENT * WIST_MATERIAL_AMBIENT;
    float diffuse = max(dot(normal, light_dir), 0.0) * WIST_MATERIAL_DIFFUSE;
    vec3 median_dir = normalize(light_dir + eye_dir);
    float specular = 0;
    if (diffuse > 0)
        specular = pow(max(dot(median_dir, normal), 0.0), WIST_MATERIAL_SHININESS) * WIST_MATERIAL_SPECULAR;

    return vec4(frag_color * (ambient + diffuse + specular), 1.0);
}

vec3 wist_applyAllLights(vec3 frag_pos, vec3 frag_normal, vec3 frag_color)
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < 8; i++)
    {
        if (WIST_LIGHTS[i].w == 0.0)
            break;
        color += wist_phongLighting(WIST_LIGHTS[i], frag_pos, frag_normal, frag_color);
    }
    return color;
}