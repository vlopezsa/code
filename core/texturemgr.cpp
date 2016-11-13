#include "texturemgr.h"
#include "vector3.h"

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}

uint32_t TextureManager::addTextureFromFile(const char * strFile)
{
    /* If the file is already loaded, just return its current ID */
    if (texFileMap.find(strFile) != texFileMap.end())
        return texFileMap[strFile];

    /* ID assigned to each texture is its position inside our img list */
    uint32_t curID = (uint32_t)imgList.size();

    Image *img = new Image();

    if (img->LoadFromFile(strFile))
    {
        delete img;
        return TEXTURE_INVALID;
    }

    imgList.push_back(img);

    texFileMap[strFile] = curID;

    return curID;
}

uint32_t TextureManager::addTextureFromImg(Image * img, const char *texName)
{
    if (!texName)
        return TEXTURE_INVALID;

    /* If the file is already loaded, just return its current ID */
    if (texFileMap.find(texName) != texFileMap.end())
        return texFileMap[texName];

    /* ID assigned to each texture is its position inside our img list */
    uint32_t curID = (uint32_t)imgList.size();

    if (!img)
        return TEXTURE_INVALID;

    imgList.push_back(img);

    texFileMap[texName] = curID;

    return curID;
}

uint32_t TextureManager::getTexturId(char * strFile)
{
    if (texFileMap.find(strFile) != texFileMap.end())
        return texFileMap[strFile];

    return TEXTURE_INVALID;
}

Image * TextureManager::getTextureImage(char * strFile)
{
    if (texFileMap.find(strFile) != texFileMap.end())
        return imgList[texFileMap[strFile]];

    return nullptr;
}

Image * TextureManager::getTextureImage(uint32_t ID)
{
    if (ID >= imgList.size())
        return nullptr;

    return imgList[ID];
}


Color3 TextureManager::sampleTextureImage(uint32_t ID, Vector2 &uvIn) {
    Color3 color;

    Image *img = getTextureImage(ID);

    if (!img)
        return color;

    Vector2 uv = uvIn;

    // Wrap UV coordinates
    if (uv.x > 1.0 || uv.x < -1.0f)
    {
        uv.x = fmodf(uv.x, 1.0f);
    }

    if (uv.y > 1.0 || uv.y < -1.0f)
    {
        uv.y = fmodf(uv.y, 1.0f);
    }

    if (uv.x < 0)
    {
        uv.x = 1.0f + uv.x;
    }

    if (uv.y < 0)
    {
        uv.y = 1.0f + uv.y;
    }

    uint32_t x, y;

    x = (uint32_t)(uv.x*(float)img->Width);
    y = (uint32_t)(uv.y*(float)img->Height);

    uint32_t offset = y*img->Width + x;

    offset *= 3;

    color.r = (float)img->Data[offset + 2] / 255.0f;
    color.g = (float)img->Data[offset + 1] / 255.0f;
    color.b = (float)img->Data[offset + 0] / 255.0f;

    return color;
}