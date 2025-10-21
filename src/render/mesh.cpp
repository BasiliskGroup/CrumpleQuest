#include "render/mesh.h"

/**
 * @brief Construct a new Mesh object from a model
 * 
 * @param modelPath The path to the model to load
 */
Mesh::Mesh(const std::string modelPath, bool generateUV, bool generateNormals) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs | (aiProcess_GenSmoothNormals & generateNormals) | (aiProcess_GenUVCoords & generateUV));

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { return; }

    unsigned int vertexOffset = 0;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];

        if (!mesh->HasPositions()) { continue; }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            if (mesh->HasPositions()) {
                vertices.push_back(mesh->mVertices[i].x);
                vertices.push_back(mesh->mVertices[i].y);
                vertices.push_back(mesh->mVertices[i].z);
            }
            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            }
            if (mesh->HasNormals()) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            }
        }

        // Process faces (indices)
        for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
            aiFace face = mesh->mFaces[k];
            for (unsigned int l = 0; l < face.mNumIndices; l++) {
                indices.push_back(face.mIndices[l] + vertexOffset);
            }
        }

        // Update the offset for the next mesh
        vertexOffset += mesh->mNumVertices;
    }

    importer.FreeScene();
}