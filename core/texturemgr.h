#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "image.h"

#define TEXTURE_INVALID 0xFFFFFFFFU

class TextureManager
{
private:
    std::vector<Image> imgList;
    std::unordered_map<std::string, unsigned int> texFileMap;

public:
    TextureManager();
    ~TextureManager();

    unsigned int addTextureFromFile(const char *strFile);

    unsigned int getTexturId(char *strFile);

    const Image *getTextureImage(char *strFile);

    const Image *getTextureImage(unsigned int ID);
};
