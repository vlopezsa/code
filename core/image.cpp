#include <exception>
#include <set>

#include <FreeImage.h>
#include "image.h"

static std::set<ImagePixelFormat> ImgPixelFormat_Set(__IMGPIXFMT_DEF);

void Image::_allocResources()
{
    if (!_Width)
    {
        throw std::exception("Invalid image width");
    }

    if (!_Height)
    {
        throw std::exception("Invalid image height");
    }

    if(!_bpp)
    {
        throw std::exception("Invalid image BPP");
    }

    if(ImgPixelFormat_Set.find(_Format) == ImgPixelFormat_Set.end())
    {
        throw std::exception("Invalid image pixel format");
    }

    _fiBitmap = FreeImage_Allocate(_Width, _Height, _bpp);

    if (!_fiBitmap)
    {
        throw std::exception("Unable to allocate memory");
    }

    data = FreeImage_GetBits(_fiBitmap);
    _Pitch = FreeImage_GetPitch(_fiBitmap);
}

void Image::_deallocResources()
{
    if (_fiBitmap)
    {
        FreeImage_Unload(_fiBitmap);
        _fiBitmap = NULL;
        data = NULL;
    }
}

Image::Image() :
    Width(_Width),
    Height(_Height),
    bpp(_bpp),
    Format(_Format),
    Pitch(_Pitch)
{
    this->_Width  = 0;
    this->_Height = 0;
    this->_bpp    = 0;
    this->_Format = Img_Pixel_RGB;

    data = NULL;
    _fiBitmap = NULL;

    this->_Pitch = 0;
}

Image::Image(const Image & img) :
    Width(_Width),
    Height(_Height),
    bpp(_bpp),
    Format(_Format),
    Pitch(_Pitch)
{
    this->_Width = img.Width;
    this->_Height = img.Height;
    this->_bpp = img.bpp;
    this->_Format = img.Format;

    try
    {
        _allocResources();

        unsigned char *out = getPixelData();


        memcpy(out,
            ((Image *)&img)->getPixelData(), // Not cool :(
            this->Height*this->Pitch);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

Image::Image(unsigned int newWidth, unsigned int newHeight, ImagePixelFormat format, unsigned int newBpp) :
    Width(_Width),
    Height(_Height),
    bpp(_bpp),
    Format(_Format),
    Pitch(_Pitch)
{
    this->_Width = newWidth;
    this->_Height = newHeight;
    this->_bpp = newBpp;
    this->_Format = format;

    data = NULL;
    _fiBitmap = NULL;

    try
    {
        _allocResources();
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

Image & Image::operator=(Image & img)
{
    _deallocResources();

    this->_Width = img.Width;
    this->_Height = img.Height;
    this->_bpp = img.bpp;
    this->_Format = img.Format;

    try
    {
        _allocResources();

        unsigned char *out = getPixelData();

        memcpy(out,
            ((Image *)&img)->getPixelData(), // Not cool :(
            this->Height*this->Pitch);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

Image::~Image()
{
    _deallocResources();
}

int Image::LoadFromFile(const char * file)
{
    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;

    unsigned int   bpp;

    _deallocResources();

    format = FreeImage_GetFileType(file);
    if (format == FIF_UNKNOWN)
        format = FreeImage_GetFIFFromFilename(file);

    if (format == FIF_UNKNOWN)
        return -1;

    if (FreeImage_FIFSupportsReading(format))
        this->_fiBitmap = FreeImage_Load(format, file, 0);

    if (this->_fiBitmap == NULL)
        return -1;

    bpp = FreeImage_GetBPP(this->_fiBitmap);

    if (bpp != 24)
    {
        FIBITMAP *fiConv = NULL;

        fiConv = FreeImage_ConvertTo24Bits(this->_fiBitmap);

        FreeImage_Unload(this->_fiBitmap);
        this->_fiBitmap = fiConv;
    }

    this->_bpp    = bpp;
    this->_Width  = FreeImage_GetWidth(this->_fiBitmap);
    this->_Height = FreeImage_GetHeight(this->_fiBitmap);
    this->_Pitch  = FreeImage_GetPitch(this->_fiBitmap);
    this->_Format = ImagePixelFormat::Img_Pixel_RGB;

    data = FreeImage_GetBits(this->_fiBitmap);

    return 0;
}

int Image::SaveToFile(const char * fileName)
{
    FREE_IMAGE_FORMAT fiFormat;

    if (!_fiBitmap)
        return -1;

    fiFormat = FreeImage_GetFIFFromFilename(fileName);

    if (fiFormat == FIF_UNKNOWN)
        fiFormat = FIF_JPEG;

    if (!FreeImage_Save(fiFormat, _fiBitmap, fileName, 0))
        return -1;

    return 0;
}

