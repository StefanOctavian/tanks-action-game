#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include "core/gpu/mesh.h"
#include "core/gpu/shader.h"
#include "meshplusplus.h"
#include "material.h"

// to whoever wrote gfxc framework:
// seriously, did you never learn to add parantheses around macro definitions?
#pragma warning(disable: 4005)  // disable macro redefinition warning
#define PATH_JOIN(...) (text_utils::Join(std::vector<std::string>{__VA_ARGS__}, std::string(1, PATH_SEPARATOR)))

namespace engine
{
    class Assets
    {
    public:
        static void LoadMesh(const std::string &name, const std::string &fileLocation, const std::string &fileName)
        {
            MeshPlusPlus *mesh = new MeshPlusPlus(name);
            mesh->LoadMesh(PATH_JOIN(lookupDirectory, fileLocation.c_str()), fileName.c_str());
            meshes[name] = mesh;
        }

        static void AddPath(const std::string &name, const std::string &path)
        {
            paths[name] = PATH_JOIN(lookupDirectory, path.c_str());
        }

        static void LoadShader(const std::string &name, const std::string &vertexShader,
                               const std::string &fragmentShader)
        {
            Shader *shader = new Shader(name);
            shader->AddShader(paths[vertexShader], GL_VERTEX_SHADER);
            shader->AddShader(paths[fragmentShader], GL_FRAGMENT_SHADER);
            shader->CreateAndLink();
            shaders[name] = shader;
        }

        static void CreateMaterial(const std::string &name, const std::string &shaderName)
        {
            Shader *shader = shaders[shaderName];
            if (!shader) {
                std::cerr << "Shader " << shaderName << " not found";
                exit(1);
            }
            materials[name] = Material(shader);
        }

        static void LoadTexture(const std::string &name, const std::string &fileLocation, 
                                const std::string &fileName)
        {
            Texture2D *texture = new Texture2D();
            texture->Load2D(PATH_JOIN(lookupDirectory, fileLocation.c_str(), fileName).c_str());
            textures[name] = texture;
        }
        
        static std::string lookupDirectory;
        static std::unordered_map<std::string, Mesh *> meshes;
        static std::unordered_map<std::string, std::string> paths;
        static std::unordered_map<std::string, Shader *> shaders;
        static std::unordered_map<std::string, Material> materials;
        static std::unordered_map<std::string, Texture2D *> textures;
    };
}