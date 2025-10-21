#ifndef INSTANCER_H
#define INSTANCER_H

#include "util/includes.h"
#include "render/shader.h"
#include "render/vbo.h"
#include "render/vao.h"
#include "render/ebo.h"
#include "render/mesh.h"

template <typename T>
class Instancer {
    private:
        Shader* shader;
        Mesh* mesh;
        VBO* vbo;
        VAO* vao;
        EBO* ebo;

        unsigned int instanceVBO;
        unsigned int capacity;
        unsigned int size;
        std::vector<T> instanceData;

        void uploadInstanceData();
        void resize();

    public:
        Instancer(Shader* shader, Mesh* mesh, std::vector<std::string> modelFormat, std::vector<std::string> instanceFormat, unsigned int reserve=1);
        ~Instancer();

        void add(T objectData);
        void remove();

        void render();
};

#include "instancer.tpp"

#endif
