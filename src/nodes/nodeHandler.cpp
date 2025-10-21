#include "nodes/nodeHandler.h"

NodeHandler::NodeHandler() {

}

NodeHandler::~NodeHandler() {

}

void NodeHandler::add(Node* node) {
    entities3d.push_back(node);
}

void NodeHandler::add(Node2D* node) {
    entities2d.push_back(node);
}

void NodeHandler::render() {
    for (Node* node : entities3d) {
        node->render();
    }
    for (Node2D* node : entities2d) {
        node->render();
    }
}