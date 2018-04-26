#include "texturemgr.h"

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


