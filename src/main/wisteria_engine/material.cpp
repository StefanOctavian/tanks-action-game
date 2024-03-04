#include <iostream>

#include "core/managers/texture_manager.h"
#include "material.h"

using namespace engine;

Material::Material(Shader *shader): shader(shader) 
{
    if (!shader || !shader->program)
        return;
    shader->Use();
    // gfxc framework already binds the sampler2D textures, but I don't like the uniform names
    for (int i = 0; i < 16; i++) {
        std::string name = "WIST_TEXTURE_" + std::to_string(i);
        GLint loc_texture = glGetUniformLocation(shader->program, name.c_str());
        if (loc_texture >= 0)
            glUniform1i(loc_texture, i);
    }
}

Material::~Material() {}

void Material::SetInt(std::string name, int value)
{
    UniformValue uniformValue;
    uniformValue.intValue = value;
    uniforms[name] = std::make_pair(INT, uniformValue);
}

void Material::SetFloat(std::string name, float value)
{
    UniformValue uniformValue;
    uniformValue.floatValue = value;
    uniforms[name] = std::make_pair(FLOAT, uniformValue);
}

void Material::SetVec2(std::string name, glm::vec2 value)
{
    UniformValue uniformValue;
    uniformValue.vec2Value = value;
    uniforms[name] = std::make_pair(VEC2, uniformValue);
}

void Material::SetVec3(std::string name, glm::vec3 value)
{
    UniformValue uniformValue;
    uniformValue.vec3Value = value;
    uniforms[name] = std::make_pair(VEC3, uniformValue);
}

void Material::SetMat3(std::string name, glm::mat3 value)
{
    UniformValue uniformValue;
    uniformValue.mat3Value = value;
    uniforms[name] = std::make_pair(MAT3, uniformValue);
}

void Material::SetMat4(std::string name, glm::mat4 value)
{
    UniformValue uniformValue;
    uniformValue.mat4Value = value;
    uniforms[name] = std::make_pair(MAT4, uniformValue);
}

void Material::Use()
{
    if (!shader || !shader->program)
        return;

    if (wireframe) {
        glLineWidth(1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    shader->Use();

    if (texture) {
        texture->BindToTextureUnit(GL_TEXTURE0);
        GLuint loc_texture = glGetUniformLocation(shader->program, "WIST_TEXTURE_0");
        glUniform1i(loc_texture, 0);
    }

    for (auto [name, uniform] : uniforms) {
        auto [type, value] = uniform;
        GLint location = glGetUniformLocation(shader->program, name.c_str());
        if (type == INT) {
            glUniform1i(location, value.intValue);
        } else if (type == FLOAT) {
            glUniform1f(location, value.floatValue);
        } else if (type == VEC2) {
            glUniform2fv(location, 1, glm::value_ptr(value.vec2Value));
        } else if (type == VEC3) {
            glUniform3fv(location, 1, glm::value_ptr(value.vec3Value));
        } else if (type == MAT3) {
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value.mat3Value));
        } else if (type == MAT4) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value.mat4Value));
        }
    }
}



