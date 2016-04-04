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
        std::vector<unsigned int> diffuse;
        std::vector<unsigned int> normal;
        std::vector<unsigned int> specular;
        std::vector<unsigned int> mask;
    }texIdx;


public:
    Material();
    ~Material();

    int importAIMaterial(TextureManager *texMgr, aiMaterial *mat);

    unsigned int getNumTexDiffuse() { return (unsigned int)texIdx.diffuse.size(); }
    unsigned int getNumTexNormal() { return (unsigned int)texIdx.normal.size(); }
    unsigned int getNumTexSpecular() { return (unsigned int)texIdx.specular.size(); }
    unsigned int getNumTexMask() { return (unsigned int)texIdx.mask.size(); }
};

