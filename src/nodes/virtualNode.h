#ifndef VIRTUAL_ENTITY_H
#define VIRTUAL_ENTITY_H

#include "util/includes.h"
#include "render/vbo.h"
#include "render/ebo.h"
#include "render/vao.h"
#include "render/mesh.h"
#include "render/shader.h"

template<typename position_type, typename rotation_type, typename scale_type>
class VirtualNode {
    private:
        Shader* shader;
        Mesh* mesh;
        Texture* texture;
        
        VBO* vbo;
        EBO* ebo;
        VAO* vao;
        
    protected:
        position_type position;
        rotation_type rotation;
        scale_type scale;
        glm::mat4 model;

        virtual void updateModel() = 0;
        
    public:
        VirtualNode(Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale);
        ~VirtualNode();

        void render();

        void setPosition(position_type position);
        void setRotation(rotation_type rotation);
        void setScale(scale_type scale);

        position_type getPosition() const { return position; }
        rotation_type getRotation() const { return rotation; }
        scale_type getScale() const { return scale; }
};

#include "nodes/virtualNode.tpp"

#endif