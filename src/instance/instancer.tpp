#include "instance/instancer.h"

/**
  * @brief Construct a new Instancer object with given template type.
  *        Template type should be per instance data, either as a single value or as a struct. 
  * 
  * @tparam T Template type for the per instance data (i.e. struct<mat4x4, int>)
  * @param shader The shader to render this instancer with
  * @param mesh The mesh to instantiate 
  * @param modelFormat 
  * @param instanceFormat 
  * @param reserve 
  */
template <typename T>
Instancer<T>::Instancer(Shader* shader, Mesh* mesh, std::vector<std::string> modelFormat, std::vector<std::string> instanceFormat, unsigned int reserve) : shader(shader), mesh(mesh), capacity(reserve), size(0) {
    // Get the object vbo and ebo
    vbo = new VBO(mesh->getVertices());
    ebo = new EBO(mesh->getIndices());
    vao = new VAO(shader, vbo, ebo);

    // Create a buffer for all instance data
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, reserve * sizeof(T), nullptr, GL_DYNAMIC_DRAW);

    // Reserve the CPU vector to match buffer
    instanceData.reserve(reserve);

    // Bind all vao components
    vao->bindBuffer(vbo, ebo, modelFormat);
    // Bind the Instance
    vao->bind();
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    vao->bindAttributes(instanceFormat, 1);
}

/**
 * @brief Destroy the Instancer object
 * 
 * @tparam T 
 */
template <typename T>
Instancer<T>::~Instancer() {
    delete vao;
    delete vbo;
    delete ebo;
    glDeleteBuffers(1, &instanceVBO);
}

/**
 * @brief Helper to upload data from the CPU vector to the GPU
 * 
 * @tparam T 
 */
template <typename T>
void Instancer<T>::uploadInstanceData()
{
    // Bind for use
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // Get a pointer to the GPU data for this frame
    void* bufferPointer = glMapBufferRange(GL_ARRAY_BUFFER, 0, size * sizeof(T), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    // Copy data from CPU to GPU
    memcpy(bufferPointer, instanceData.data(), size * sizeof(T));
    // Unmap to flush data
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

/**
 * @brief Double the capacity of the buffer. Does not copy data to reallocation
 * 
 * @tparam T 
 */
template <typename T>
void Instancer<T>::resize() {
    // Double the capacity (O(1) amortized!)
    capacity *= 2;
    
    // Reallocate the buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(T), nullptr, GL_DYNAMIC_DRAW);
}

/**
 * @brief Add an object to the instancer
 * 
 * @tparam T 
 * @param objectData The data for the object to add. Should be in the format of template type.
 */
template <typename T>
void Instancer<T>::add(T objectData) {
    // Add new object
    instanceData.push_back(objectData);
    size++;

    // Resize if beyond capacity
    if (size > capacity) {
        resize();
    }

    // Upload the new data to the GPU
    uploadInstanceData();
}

/**
 * @brief Render the all instances
 * 
 * @tparam T 
 */
template <typename T>
void Instancer<T>::render() {
    vao->render(size);
}