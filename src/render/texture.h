#ifndef TEXTURE_H
#define TEXTURE_H

#include "render/image.h"

class Texture {
    private:
        unsigned int id;

    public:
        Texture(Image* image);
        ~Texture();

        void bind();        
        void setFilter(unsigned int magFilter, unsigned int minFilter);
        void setWrap(unsigned int wrap);

        unsigned int getID() { return id; }
};

#endif