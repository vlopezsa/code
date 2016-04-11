#include <FreeImage.h>

#include "ImageLoader.h"

#include <stdio.h>

void ImageLoader::Init()
{
    FreeImage_Initialise();
}

void ImageLoader::Close()
{
    FreeImage_DeInitialise();
}

uint8_t *ImageLoader::LoadFile(char *sFileName, int *out_Width, int *out_Height)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
    FIBITMAP *fiBitmap = NULL;

	uint8_t *Data = NULL;
    unsigned int   bpp;

    format = FreeImage_GetFileType(sFileName);
    if (format == FIF_UNKNOWN)
        format = FreeImage_GetFIFFromFilename(sFileName);

    if (format == FIF_UNKNOWN)
        return NULL;
    
    if (FreeImage_FIFSupportsReading(format))
        fiBitmap = FreeImage_Load(format, sFileName, 0);

    if (fiBitmap == NULL)
        return NULL;

    bpp=FreeImage_GetBPP(fiBitmap);

    if (bpp != 24)
    {
        FIBITMAP *fiConv=NULL;

        fiConv = FreeImage_ConvertTo24Bits(fiBitmap);

        FreeImage_Unload(fiBitmap);
        fiBitmap = fiConv;
    }

    Data = FreeImage_GetBits(fiBitmap);

    *out_Width  = FreeImage_GetWidth (fiBitmap);
    *out_Height = FreeImage_GetHeight(fiBitmap);

	return Data;
}

