#include "render/vbo.h"

/**
 * @brief Construct a new VBO from the data provided at the given pointer
 *
 * @param data A pointer to the data to populate the VBO.
 * @param size The size of the data provided in bytes.
 * @param drawType Specifies how OpenGL should store/write to buffer data.
 *                 GL_STATIC_DRAW by defult. 
 *                 May also be GL_DYNAMIC_DRAW or GL_STREAM_DRAW.
 */
VBO::VBO(const void* data, unsigned int size, unsigned int drawType): size(size) {
    // Create one buffer, and update VBO with the buffer ID
    glGenBuffers(1, &ID);
    // Bind the vbo to start working on it
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    // Now, we can add our vertex data to the VBO
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    // Unbind the buffer for safety
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * @brief Destroy the VBO::VBO object
 * 
 */
VBO::~VBO() {
    glDeleteBuffers(1, &ID);
}

/**
 * @brief Binds this VBO for use
 * 
 */
void VBO::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

/**
 * @brief Unbinds this VBO
 * 
 */
void VBO::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * @brief Get the size of the current data in bytes
 * 
 * @return Size in bytes 
 */
unsigned int VBO::getSize() {
    return size;
}

/**
 * @brief Writes an array of data to an existing allocation of the VBO.
 * 
 * @param data A pointer to the array of data to be writen
 * @param size The size of the data in bytes
 * @param offset The location in bytes to start writing
 */
void VBO::write(const void* data, unsigned int size, unsigned int offset) {
    // Bind the vbo to start working on it
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    // Write the data 
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    // Unbind for safety
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * @brief Writes a vector of data to an existing allocation of the VBO.
 * 
 * @param data A reference to the vecotre of data to be writen
 * @param offset The location in bytes to start writing
 */
template<typename T>
void VBO::write(const std::vector<T>& data, unsigned int offset) {
    write(data.data(), data.size() * sizeof(T), offset);
}