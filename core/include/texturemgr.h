#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "image.h"
#include "vector2.h"
#include "vector3.h"

#define TEXTURE_INVALID 0xFFFFFFFFU

class TextureManager
{
private:
    std::vector<Image *> imgList;
    std::unordered_map<std::string, uint32_t> texFileMap;

public:
    TextureManager();
    ~TextureManager();

    uint32_t addTextureFromFile(const char *strFile);

    uint32_t addTextureFromImg(Image *img, const char *texName);

    uint32_t getTexturId(char *strFile);

    Image *getTextureImage(char *strFile);

    Image *getTextureImage(uint32_t ID);

    uint32_t getNumTextures() { return (uint32_t)imgList.size(); }

    Color3 sampleTextureImage(uint32_t ID, Vector2 &uv);
};
