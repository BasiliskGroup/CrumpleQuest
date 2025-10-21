#include "render/ebo.h"

/**
 * @brief Construct a new EBO from the data provided at the given pointer
 *
 * @param data A pointer to the data to populate the EBO.
 * @param size The size of the data provided in bytes.
 * @param drawType Specifies how OpenGL should store/write to buffer data.
 *                 GL_STATIC_DRAW by defult. 
 *                 May also be GL_DYNAMIC_DRAW or GL_STREAM_DRAW.
 */
EBO::EBO(const void* data, unsigned int size, unsigned int drawType): size(size) {
    // Create one buffer, and update EBO with the buffer ID
    glGenBuffers(1, &ID);
    // Bind the ebo to start working on it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    // Now, we can add our vertex data to the EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    // Unbind the buffer for safety
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/**
 * @brief Destroy the EBO::EBO object
 * 
 */
EBO::~EBO() {
    glDeleteBuffers(1, &ID);
}

/**
 * @brief Binds this EBO for use
 * 
 */
void EBO::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

/**
 * @brief Unbinds this EBO
 * 
 */
void EBO::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/**
 * @brief Get the size of the current data in bytes
 * 
 * @return Size in bytes 
 */
unsigned int EBO::getSize() {
    return size;
}

/**
 * @brief Writes an array of data to an existing allocation of the EBO.
 * 
 * @param data A pointer to the array of data to be writen
 * @param size The size of the data in bytes
 * @param offset The location in bytes to start writing
 */
void EBO::write(const void* data, unsigned int size, unsigned int offset) {
    // Bind the ebo to start working on it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    // Write the data 
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
    // Unbind for safety
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/**
 * @brief Writes a vector of data to an existing allocation of the EBO.
 * 
 * @param data A reference to the vecotre of data to be writen
 * @param offset The location in bytes to start writing
 */
template<typename T>
void EBO::write(const std::vector<T>& data, unsigned int offset) {
    write(data.data(), data.size() * sizeof(T), offset);
}