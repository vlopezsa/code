#pragma once

#include <FreeImage.h>

#define __IMGPIXFMT_DEF { Img_Pixel_Mono, Img_Pixel_RGB, Img_Pixel_RGBA, Img_Pixel_RGBF, Img_Pixel_RGBAF}

enum ImagePixelFormat __IMGPIXFMT_DEF;


class Image
{
private:
    uint8_t *_data;
    uint32_t  _Width;
    uint32_t  _Height;
    uint32_t  _bpp;
    ImagePixelFormat _Format;

    uint32_t   _Pitch;

    FIBITMAP *_fiBitmap = NULL;

public:
    const uint32_t &Width;  // Image width, in pixels
    const uint32_t &Height; // Image height, in pixels
    const uint32_t &bpp;    // Bits per pixel
    const ImagePixelFormat &Format; // Pixel format

    const uint32_t &Pitch;
    uint8_t * const &Data;

private:
    void _allocResources();
    void _deallocResources();

public:
    Image();
    Image(const Image &img);

    Image(uint32_t newWidth, uint32_t newHeight, ImagePixelFormat format, uint32_t newBpp);

    Image &operator = (const Image &img);

    ~Image();

    int LoadFromFile(const char *fileName);
    int SaveToFile(const char *fileName);

    uint8_t *getData() { return _data; }

};

