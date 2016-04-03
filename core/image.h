#pragma once

#include <FreeImage.h>

#define __IMGPIXFMT_DEF { Img_Pixel_Mono, Img_Pixel_RGB, Img_Pixel_RGBA}

enum ImagePixelFormat __IMGPIXFMT_DEF;

class Image
{
private:
    unsigned char *data;
    unsigned int  _Width;
    unsigned int  _Height;
    unsigned int  _bpp;
    ImagePixelFormat _Format;

    unsigned int   _Pitch;

    FIBITMAP *_fiBitmap = NULL;

public:
    const unsigned int &Width;  // Image width, in pixels
    const unsigned int &Height; // Image height, in pixels
    const unsigned int &bpp;    // Bits per pixel
    const ImagePixelFormat &Format; // Pixel format
    const unsigned int &Pitch;

private:
    void _allocResources();
    void _deallocResources();

public:
    Image();
    Image(const Image &img);

    Image(unsigned int newWidth, unsigned int newHeight, ImagePixelFormat format, unsigned int newBpp);

    Image &operator = (Image &img);

    ~Image();

    int LoadFromFile(const char *fileName);
    int SaveToFile(const char *fileName);

    unsigned char *getPixelData() { return data; }
};

