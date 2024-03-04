#pragma once

#include "core/gpu/shader.h"
#include "core/gpu/texture2D.h"
#include "utils/glm_utils.h"

namespace engine
{
    class Material
    {
    public:
        Material(): shader(nullptr) {}
        Material(Shader *shader);
        ~Material();

        void SetInt(std::string name, int value);
        void SetFloat(std::string name, float value);
        void SetVec2(std::string name, glm::vec2 value);
        void SetVec3(std::string name, glm::vec3 value);
        void SetMat3(std::string name, glm::mat3 value);
        void SetMat4(std::string name, glm::mat4 value);

        void Use();

        Shader *shader;
        Texture2D *texture = nullptr;
        bool wireframe = false; 

    private:
        enum UniformType
        {
            INT, FLOAT,
            VEC2, VEC3,
            MAT3, MAT4
        };
        union UniformValue
        {
            int intValue;
            float floatValue;
            glm::vec2 vec2Value; glm::vec3 vec3Value;
            glm::mat3 mat3Value; glm::mat4 mat4Value;
        };
        std::unordered_map<std::string, std::pair<UniformType, UniformValue>> uniforms;
    };
}