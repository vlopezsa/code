#pragma once

#include <vector>
#include <string>

#include <assimp\material.h>

#include "graphics.h"
#include "image.h"
#include "texturemgr.h"

class Material
{
public :
    std::string strName;

    struct {
        Vector3 diffuse;
        Vector3 emissive;
        Vector3 ambient;
        Vector3 specular;
    }Color;

    struct {
        std::vector<uint32_t> diffuse;
        std::vector<uint32_t> normal;
        std::vector<uint32_t> specular;
        std::vector<uint32_t> mask;
    }texIdx;


public:
    Material();
    ~Material();

    int importAIMaterial(TextureManager *texMgr, aiMaterial *mat);

    uint32_t getNumTexDiffuse() { return (uint32_t)texIdx.diffuse.size(); }
    uint32_t getNumTexNormal() { return (uint32_t)texIdx.normal.size(); }
    uint32_t getNumTexSpecular() { return (uint32_t)texIdx.specular.size(); }
    uint32_t getNumTexMask() { return (uint32_t)texIdx.mask.size(); }
};

