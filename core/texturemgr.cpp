#include "texturemgr.h"

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}

unsigned int TextureManager::addTextureFromFile(const char * strFile)
{
    /* If the file is already loaded, just return its current ID */
    if (texFileMap.find(strFile) != texFileMap.end())
        return texFileMap[strFile];

    /* ID assigned to each texture is its position inside our img list */
    unsigned int curID = (unsigned int)imgList.size();

    imgList.resize(curID+1);

    if (imgList[curID].LoadFromFile(strFile))
    {
        imgList.pop_back();
        return TEXTURE_INVALID;
    }

    texFileMap[strFile] = curID;

    return curID;
}

unsigned int TextureManager::getTexturId(char * strFile)
{
    if (texFileMap.find(strFile) != texFileMap.end())
        return texFileMap[strFile];

    return TEXTURE_INVALID;
}

const Image * TextureManager::getTextureImage(char * strFile)
{
    if (texFileMap.find(strFile) != texFileMap.end())
        return &imgList[texFileMap[strFile]];

    return nullptr;
}

const Image * TextureManager::getTextureImage(unsigned int ID)
{
    if (ID >= imgList.size())
        return nullptr;

    return &imgList[ID];
}


