#ifndef MESH_H
#define MESH_H

#include "util/includes.h"

class Mesh {
    private:
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

    public:
        Mesh(const std::string modelPath, bool generateUV=false, bool generateNormals=false);

        std::vector<float>& getVertices() { return vertices; }
        std::vector<unsigned int>& getIndices() { return indices; }
};

#endif