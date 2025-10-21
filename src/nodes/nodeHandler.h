#ifndef ENTITY_HANDLER_H
#define ENTITY_HANDLER_H

#include "util/includes.h"
#include "nodes/node.h"
#include "nodes/node2d.h"


class NodeHandler {
    private:
        std::vector<Node*> entities3d;
        std::vector<Node2D*> entities2d;

    public:
        NodeHandler();
        ~NodeHandler();

        void render();

        void add(Node* node);
        void add(Node2D* node);
};

#endif