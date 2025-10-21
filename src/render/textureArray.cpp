#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "render/textureArray.h"

/**
 * @brief Construct a new Texture Array object
 * 
 * @param width The desired width of the images in the texture array. Will resize images if needed.
 * @param height The desired height of the images in the texture array. Will resize images if needed.
 * @param images A vector of image pointers
 * @param capacity The inital capacity of the array for preallocation. Will use size of images if not given.
 */
TextureArray::TextureArray(unsigned int width, unsigned int height, std::vector<Image*> images, unsigned int capacity): width(width), height(height), images(images) {
    this->capacity = glm::max((unsigned int)images.size(), capacity);
    glGenTextures(1, &id);
    generate();
}

/**
 * @brief Destroy the Texture Array object. Releases gl texture array. Leaves images intact.
 * 
 */
TextureArray::~TextureArray() {
    glDeleteTextures(1, &id);
}

/**
 * @brief Generates the array and uploads all the image data from the image vector to this texture array
 * 
 */
void TextureArray::generate() {
    // Generate new array
    bind();
    
    // Set the filter to liear by default
    setFilter(GL_LINEAR, GL_LINEAR);
    setWrap(GL_REPEAT);
    
    // Set up the array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, capacity, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Upload the data for each image
    for (unsigned int i = 0; i < images.size(); i++) {
        uploadImage(images.at(i), i);
    }
}

/**
 * @brief Uploads an image to the texture array. Assumes the given position slot is allocated
 * 
 */
void TextureArray::uploadImage(Image* image, unsigned int position) {
    // Resize the image
    unsigned char* data = new unsigned char[width * height * 4];
    stbir_resize_uint8_linear(image->getData(), image->getWidth(), image->getHeight(), 0, data, width, height, 0, STBIR_RGBA);

    // Add the resized image data to the texture array
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, position, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Free the resized image data
    delete[] data;
}

/**
 * @brief Adds a new image to the texture array.
 * 
 * @param image Pointer to the image to add.
 * 
 * @return unsigned int of the location of the image in the array. 
 */
unsigned int TextureArray::add(Image* image) {
    images.push_back(image);
    
    if (images.size() > capacity) {
        capacity *= 2;
        generate();
    }
    else {
        uploadImage(image, images.size() - 1);
    }

    return images.size() - 1;
}

/**
 * @brief Binds this texture array for use.
 * 
 */
void TextureArray::bind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

/**
 * @brief Unbinds this texture array (binds 0)
 * 
 */
void TextureArray::unbind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

/**
 * @brief Sets the filter of all the images in the texture array
 * 
 * @param magFilter GL_LINEAR
 * @param minFilter GL_LINEAR
 */
void TextureArray::setFilter(unsigned int magFilter, unsigned int minFilter) {
    // Bind the texture array
    bind();
    // Set filter
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
}

/**
 * @brief Sets the wrap method for all images in the texture array
 * 
 * @param wrap GL_REPEAT 
 */
void TextureArray::setWrap(unsigned int wrap) {
    // Bind the texture array
    bind();
    // Set wrap
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrap);	
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrap);
}