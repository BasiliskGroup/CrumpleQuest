#include "render/shader.h"

/**
 * @brief Get the component count of an OpenGL type. Not an exhaustive function, only valid for BOOL, INT, and FLOAT type. 
 * 
 * @param type GLenum of the type
 * @return GLint 
 */
inline GLint getGLTypeComponentCount(GLenum type) {
    switch (type) {
        case GL_BOOL:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT: return 1;

        case GL_BOOL_VEC2:
        case GL_INT_VEC2:
        case GL_FLOAT_VEC2: return 2;

        case GL_BOOL_VEC3:
        case GL_INT_VEC3:
        case GL_FLOAT_VEC3: return 3;

        case GL_BOOL_VEC4:
        case GL_INT_VEC4:
        case GL_FLOAT_VEC4: return 4;
    }

    return 0;
}

/**
 * @brief Get the size in bytes of an OpenGL type. Not an exhaustive function, only valid for BOOL, INT, and FLOAT type. 
 * 
 * @param type GLenum of the type
 * @return GLsizei 
 */
inline GLsizei getGLTypeSize(GLenum type) {
    switch (type) {
        case GL_BOOL:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT: return 4;

        case GL_BOOL_VEC2:
        case GL_INT_VEC2:
        case GL_FLOAT_VEC2: return 8;

        case GL_BOOL_VEC3:
        case GL_INT_VEC3:
        case GL_FLOAT_VEC3: return 12;

        case GL_BOOL_VEC4:
        case GL_INT_VEC4:
        case GL_FLOAT_VEC4: return 16;
    }

    return 0;
}

/**
 * @brief Read a file as a c-string
 * 
 * @param path The absolute or relative path of the file to read
 * @return const char* 
 */
std::string loadFile(const char* path) {
    // Set out string and stream
    std::string content;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    // Attempt to load the file and read to string
    try {
        file.open(path);
        std::stringstream vertexStream, fragmentStream;
        vertexStream << file.rdbuf();
        content = vertexStream.str();
        file.close();
    }
    catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    return content;
}

/**
 * @brief Compiles a shader from source code and returns the ID.
 * 
 * @param source C-String containg the shader source code
 * @param shaderType The type of shader. May be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 * @return unsigned int of the shader ID
 */
unsigned int loadShader(std::string source, unsigned int shaderType) {
    int success;
    char infoLog[512];

    // Load Shader
    unsigned int shader = glCreateShader(shaderType);
    const char* sourceCode = source.c_str();
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

/**
 * @brief Links a shader program given a vertex and fragment shader. Returns program ID. 
 * 
 * @param vertex ID of the vertex shader
 * @param fragment ID of the fragment shader
 * @return unsigned int 
 */
unsigned int loadProgram(unsigned int vertex, unsigned int fragment) {
    int success;
    char infoLog[512];

    // Shader program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram (program);

    // Check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    return program;
}

/**
 * @brief Construct a new Shader object from vertex and fragment source
 * 
 * @param vertexPath 
 * @param fragmentPath 
 */
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    //  Load the source code
    std::string vertexShaderSource   = loadFile(vertexPath);
    std::string fragmentShaderSource = loadFile(fragmentPath);

    // Compile shaders from source
    unsigned int vertex   = loadShader(vertexShaderSource,   GL_VERTEX_SHADER);
    unsigned int fragment = loadShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Shader program
    ID = loadProgram(vertex, fragment);

    // Release hanging shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Get all of the active attributes in the shader for VAO use
    loadAttributes();
}

/**
 * @brief Get all of the active attributes in the shader and saves them on the shader
 * 
 */
void Shader::loadAttributes() {
    GLint nAttributes;
    GLint size; 
    GLenum type; 
    const GLsizei bufSize = 256;
    GLchar name[bufSize];
    GLsizei length; 

    glGetProgramiv(ID, GL_ACTIVE_ATTRIBUTES, &nAttributes);

    stride = 0;
    for (GLint i = 0; i < nAttributes; i++) {
        glGetActiveAttrib(ID, (GLuint)i, bufSize, &length, &size, &type, name);
        attributes.push_back({name, i, getGLTypeComponentCount(type), type, stride});
        stride += getGLTypeSize(type);
    }
}

/**
 * @brief Destroy the Shader object
 * 
 */
Shader::~Shader() { 
    glDeleteProgram(ID);
}

/**
 * @brief Uses the shader program for rendering
 * 
 */
void Shader::use() { 
    glUseProgram(ID); 
}

/**
 * @brief Binds a texture to the shader
 * 
 * @param name Name of the texture on the shader
 * @param texture Pointer to the texture to bind
 * @param slot Slot to bind the texutre to [1-16]
 */
void Shader::bind(const char* name, Texture* texture, unsigned int slot) {
    use();
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->getID());
    setUniform(name, (int)slot);   
}

/**
 * @brief Binds a texture arraY to the shader
 * 
 * @param name Name of the texture on the shader
 * @param textureArray Pointer to the texture array to bind
 * @param slot Slot to bind the texutre to [0-15]
 */
void Shader::bind(const char* name, TextureArray* textureArray, unsigned int slot) {
    use();
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray->getID());
    setUniform(name, (int)slot);
}

/**
 * @brief Get the location of a uniform on this shader. 
 * 
 * @param name C-String name of the uniform 
 * @return int 
 */
int Shader::getUniformLocation(const char* name){
    return glGetUniformLocation(ID, name);
}

/**
 * @brief Set a float uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, float value) { 
    use();
    glUniform1f(getUniformLocation(name), value); 
}

/**
 * @brief Set a double uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, double value) {
    Shader::setUniform(name, (float)value);
}

/**
 * @brief Set an int uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, int value) { 
    use();
    glUniform1i(getUniformLocation(name), value); 
}

/**
 * @brief Set a matrix uniform value
 * 
 * @param name Name of the uniform on the shader
 * @param value Value to set the uniform
 */
void Shader::setUniform(const char* name, glm::mat4 value) { 
    use();
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));  
}