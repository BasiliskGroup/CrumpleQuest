#include "nodes/virtualNode.h"

/**
 * @brief Construct a new Node object
 * 
 * @param shader 
 * @param mesh 
 * @param texture 
 * @param position 
 * @param rotation
 * @param scale 
 */
template<typename position_type, typename rotation_type, typename scale_type>
VirtualNode<position_type, rotation_type, scale_type>::VirtualNode(Shader* shader, Mesh* mesh, Texture* texture, position_type position, rotation_type rotation, scale_type scale):
    shader(shader), mesh(mesh), texture(texture), position(position), rotation(rotation), scale(scale){

    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);
}

/**
 * @brief Destroy the Node object. Release the vbo, ebo, and vao
 * 
 */
template<typename position_type, typename rotation_type, typename scale_type>
VirtualNode<position_type, rotation_type, scale_type>::~VirtualNode() {
    delete vao;
    delete vbo;
    delete ebo;
}

/**
 * @brief Render the vao on this node
 * 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<position_type, rotation_type, scale_type>::render() {
    shader->bind("uTexture", texture, 0);
    shader->setUniform("uModel", model);
    vao->render();
}

/**
 * @brief Update the position of the node
 * 
 * @param position 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<position_type, rotation_type, scale_type>::setPosition(position_type position) {
    this->position = position;
    updateModel();
}

/**
 * @brief Update the rotation of the node
 * 
 * @param rotation 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<position_type, rotation_type, scale_type>::setRotation(rotation_type rotation) {
    rotation = rotation;
    updateModel();
}

/**
 * @brief Update the scale of the node
 * 
 * @param scale 
 */
template<typename position_type, typename rotation_type, typename scale_type>
void VirtualNode<position_type, rotation_type, scale_type>::setScale(scale_type scale) {
    scale = scale;
    updateModel();
}
