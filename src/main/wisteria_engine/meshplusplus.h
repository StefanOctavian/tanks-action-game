#pragma once

#include "components/simple_scene.h"

#include "assimp/Importer.hpp"          // C++ importer interface
#include "assimp/postprocess.h"         // Post processing flags

// Mesh++ is a wrapper around Mesh that adds support for vertex colors
// Just because the framework is poorly designed and I need to work around it
// NOTICE: The following code is mostly copied from Mesh.h !!
// I can't just inherit from it because of the way the framework is designed
// The functions are not virtual and I can't override them
namespace engine
{
    class MeshPlusPlus : public Mesh
    {
    private:
        void InitMesh(const aiMesh* paiMesh)
        {
            const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
            const aiColor4D White(1.0f, 1.0f, 1.0f, 1.0f);

            // Create VertexFormats instead of populating different vectors
            // VertexFormat contains color information as well
            // Since most meshes have at most 1 set of vertex colors, we will only use the first one
            for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
                const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
                const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
                const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
                const aiColor4D*  pColor    = paiMesh->HasVertexColors(0) ? &(paiMesh->mColors[0][i]) : &White;

                VertexFormat vertex(glm::vec3(pPos->x, pPos->y, pPos->z),
                                    glm::vec3(pColor->r, pColor->g, pColor->b),
                                    glm::vec3(pNormal->x, pNormal->y, pNormal->z),
                                    glm::vec2(pTexCoord->x, pTexCoord->y));
                vertices.push_back(vertex);
            }

            // Init the index buffer
            for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
                const aiFace& Face = paiMesh->mFaces[i];
                indices.push_back(Face.mIndices[0]);
                indices.push_back(Face.mIndices[1]);
                indices.push_back(Face.mIndices[2]);
                if (Face.mNumIndices == 4)
                    indices.push_back(Face.mIndices[3]);
            }
        }

        bool InitFromScene(const aiScene *pScene)
        {
            meshEntries.resize(pScene->mNumMeshes);
            materials.resize(pScene->mNumMaterials);

            unsigned int nrVertices = 0;
            unsigned int nrIndices = 0;

            // Count the number of vertices and indices
            for (unsigned int i = 0 ; i < pScene->mNumMeshes ; i++)
            {
                meshEntries[i].materialIndex = pScene->mMeshes[i]->mMaterialIndex;
                meshEntries[i].nrIndices = (pScene->mMeshes[i]->mNumFaces * (glDrawMode == GL_TRIANGLES ? 3 : 4));
                meshEntries[i].baseVertex = nrVertices;
                meshEntries[i].baseIndex = nrIndices;

                nrVertices += pScene->mMeshes[i]->mNumVertices;
                nrIndices  += meshEntries[i].nrIndices;
            }

            // Reserve space in the vectors for the vertex attributes and indices
            vertices.reserve(nrVertices);
            indices.reserve(nrIndices);

            // Initialize the meshes in the scene one by one
            for (unsigned int i = 0 ; i < meshEntries.size() ; i++)
            {
                const aiMesh* paiMesh = pScene->mMeshes[i];
                InitMesh(paiMesh);
            }

            if (useMaterial && !InitMaterials(pScene))
                return false;

            buffers->ReleaseMemory();
            *buffers = gpu_utils::UploadData(vertices, indices);
            return buffers->m_VAO != 0;
        }

    public:
        MeshPlusPlus(const std::string &meshID) : Mesh(meshID) {}

        bool LoadMesh(const std::string& fileLocation,
                      const std::string& fileName)
        {
            ClearData();
            this->fileLocation = fileLocation;
            std::string file = (fileLocation + '/' + fileName).c_str();

            Assimp::Importer Importer;

            unsigned int flags = aiProcess_GenSmoothNormals | aiProcess_FlipUVs;
            if (glDrawMode == GL_TRIANGLES) flags |= aiProcess_Triangulate;

            const aiScene* pScene = Importer.ReadFile(file, flags);

            if (pScene) {
                return InitFromScene(pScene);
            }

            // pScene is freed when returning because of Importer

            printf("Error parsing '%s': '%s'\n", file.c_str(), Importer.GetErrorString());
            return false;
        }
    };
}