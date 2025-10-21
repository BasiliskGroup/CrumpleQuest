#ifndef ENTITY_H
#define ENTITY_H

#include "util/includes.h"
#include "nodes/virtualNode.h"

class Node : public VirtualNode<glm::vec3, glm::vec3, glm::vec3>{
    protected:
        void updateModel() override;
    public:
        Node(Shader* shader, Mesh* mesh, Texture* texture, glm::vec3 position={0, 0, 0}, glm::vec3 rotation={0, 0, 0}, glm::vec3 scale={1, 1, 1})
            : VirtualNode(shader, mesh, texture, position, rotation, scale) {
                updateModel();
        }
};

#endif