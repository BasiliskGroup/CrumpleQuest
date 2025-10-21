#ifndef IMAGE_H
#define IMAGE_H

#include "util/includes.h"

class Image {
    private:
        unsigned char* data;
        int width;
        int height;
        int nChannels;
    
    public:
        Image(std::string file);
        ~Image();
        
        unsigned char* getData() { return data; }
        int getWidth() { return width; }
        int getHeight() {return height; }
};

#endif