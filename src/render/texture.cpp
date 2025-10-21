#include "render/texture.h"

/**
 * @brief Construct a new Texture object from an existing Image object
 * 
 * @param image 
 */
Texture::Texture(Image* image) {
    // Create one texture, and update texture with the ID
    glGenTextures(1, &id); 
    // Bind the texture to start working on it
    glBindTexture(GL_TEXTURE_2D, id);
    // Add image data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->getData());
    // Generate Mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
}

/**
 * @brief Destroy the Texture object, release GL data
 *        Does not destroy the image.
 * 
 */
Texture::~Texture() {
    glDeleteTextures(1, &id);
}

/**
 * @brief Binds the texture to make changes or use for texture unit. Does not bind to shader.
 * 
 */
void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}

/**
 * @brief Sets the filter of all the images in the texture array
 * 
 * @param magFilter GL_LINEAR
 * @param minFilter GL_LINEAR
 */
void Texture::setFilter(unsigned int magFilter, unsigned int minFilter) {
    // Bind the texture
    bind();
    // Set filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
}

/**
 * @brief Sets the wrap method for all images in the texture array
 * 
 * @param wrap GL_REPEAT
 */
void Texture::setWrap(unsigned int wrap) {
    // Bind the texture
    bind();
    // Set wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}