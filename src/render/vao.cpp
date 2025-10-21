#include "render/vao.h"

/**
 * @brief Construct a new VAO object with a shader object, VBO, and optional EBO
 * 
 * @param shader The shader to use when rendering the vao
 * @param vertices The buffer (VBO pointer) of the verticies
 * @param indices Optional buffer (EBO pointer) of the indices
 */
VAO::VAO(Shader* shader, VBO* vertices, EBO* indices): shader(shader), vbo(vertices), ebo(indices) {
    // Create a new VAO
    glGenVertexArrays(1, &ID);

    // Bind the VBO and EBO
    bind();
    vbo->bind();
    if (ebo) { ebo->bind(); } 

    // Bind attributes
    for (auto attrib : shader->getAttributes()) {
        bindAttribute(attrib.location, attrib.count, attrib.dataType, shader->getStride(), attrib.offset);
    }
}

/**
 * @brief Destroy the VAO object and release GPU data
 * 
 */
VAO::~VAO() {
    glDeleteVertexArrays(1, &ID); 
}

/**
 * @brief Bind a VBO to this VAO with a given list of attribs. Can be used for standard vbo or instance buffer.
 * 
 * @param buffer Pointer to the vbo to bind 
 * @param attribs A vector of strings of the attribute names as they appear in the shader code
 * @param divisor Set to 0 for default. Use 1 for standard instancing. 
 */
void VAO::bindBuffer(VBO* buffer, std::vector<std::string> attribs, unsigned int divisor) {
    // Bind the VAO and VBO
    bind();
    buffer->bind();

    // Bind the given attribs
    bindAttributes(attribs, divisor);
}

/**
 * @brief Bind a VBO and EBO to this VAO with a given list of attribs. Can be used for standard vbo or instance buffer.
 * 
 * @param buffer Pointer to the vbo to bind
 * @param indices Pointer to the ebo to bind
 * @param attribs A vector of strings of the attribute names as they appear in the shader code
 * @param divisor Set to 0 for default. Use 1 for standard instancing. 
 */
void VAO::bindBuffer(VBO* buffer, EBO* indices, std::vector<std::string> attribs, unsigned int divisor) {
    // Bind the VAO, EBO, and VBO
    bind();
    buffer->bind();
    indices->bind();

    // Bind the given attribs
    bindAttributes(attribs, divisor);
}

/**
 * @brief Bind a list of attributes. Per attribute information pulled from this VAO's shader
 * 
 * @param attribs A vector of strings of the attribute names as they appear in the shader code
 * @param divisor Set to 0 for default. Use 1 for standard instancing. 
 */
void VAO::bindAttributes(std::vector<std::string> attribs, unsigned int divisor) {
    // Get the stride
    unsigned int stride = 0;
    for (auto attrib : shader->getAttributes()) {
        if (std::find(attribs.begin(), attribs.end(), attrib.name) == attribs.end()) {
            continue; 
        }

        stride += attrib.count;
    }

    // Bind attributes
    unsigned int offset = 0;
    for (auto attrib : shader->getAttributes()) {
        if (std::find(attribs.begin(), attribs.end(), attrib.name) == attribs.end()) {
            continue;
        }
        // Bind the attribute
        bindAttribute(attrib.location, attrib.count, attrib.dataType, stride * sizeof(float), offset * sizeof(float), divisor);
        // Increment the offset within this buffer
        offset += attrib.count;
    }
}

/**
 * @brief Binds an attribute on the vao.
 * 
 * @param location The location of the attribute in the shader source
 * @param count The number of components in the attribute (1 for float, 3 for vec3, etc.)
 * @param dataType OpenGL data type, such as GL_FLOAT, GL_INT
 * @param stride Space between each occurrence of the attribute in the VBO
 * @param offset Starting point of attribute in the VBO
 */
void VAO::bindAttribute(GLint location, GLint count, unsigned int dataType, unsigned int stride, unsigned int offset, unsigned int divisor) {
    bind();
    glVertexAttribPointer(location, count, GL_FLOAT, dataType, stride, (const void*)(GLintptr)offset);
    glEnableVertexAttribArray(location);
    glVertexAttribDivisor(location, divisor);
}

/**
 * @brief Binds this VAO for rendering
 * 
 */
void VAO::bind() {
    glBindVertexArray(ID);
}

/**
 * @brief Renders this VAO based on current data
 * 
 */
void VAO::render(unsigned int instanceCount) {
    // Use the shader and this VAO
    shader->use();
    bind();
    vbo->bind();
    if (ebo) { ebo->bind(); }
    
    // Choose render method based on EBO
    if (ebo) {
        int vertexCount = ebo->getSize() / sizeof(unsigned int);
        
        if (instanceCount) {
            glDrawElementsInstanced(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0, instanceCount); 
        } 
        else {
            glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
        }
    }
    else {
        int vertexCount = vbo->getSize() / (sizeof(float) * 3);

        if (instanceCount) {
            glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        }
    }
}