#include "assets.h"
#include "material.h"

using namespace engine;

std::string Assets::lookupDirectory;
std::unordered_map<std::string, Mesh *> Assets::meshes;
std::unordered_map<std::string, std::string> Assets::paths;
std::unordered_map<std::string, Shader *> Assets::shaders;
std::unordered_map<std::string, engine::Material> Assets::materials;
std::unordered_map<std::string, Texture2D *> Assets::textures;