#ifndef ENTITY2D_H
#define ENTITY2D_H

#include "util/includes.h"
#include "nodes/virtualNode.h"

class Node2D : public VirtualNode<glm::vec2, float, glm::vec2>{
    protected:
        float layer=0.0;
        void updateModel() override;

    public:
        Node2D(Shader* shader, Mesh* mesh, Texture* texture, glm::vec2 position={0, 0}, float rotation=0, glm::vec2 scale={100, 100})
            : VirtualNode(shader, mesh, texture, position, rotation, scale) {
                updateModel();
            }
};

#endif