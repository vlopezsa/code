#include "material.h"

Material::Material()
{
}

Material::~Material()
{
}

int Material::importAIMaterial(TextureManager *texMgr, aiMaterial * mat)
{
    uint32_t ntex;
    aiString  aistr;
    aiColor3D color;

    if(mat->Get(AI_MATKEY_NAME, aistr)==AI_SUCCESS)
        strName = std::string(aistr.C_Str());

    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        Color.diffuse = Vector3(color.r, color.g, color.b);

    if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
        Color.ambient = Vector3(color.r, color.g, color.b);

    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
        Color.specular = Vector3(color.r, color.g, color.b);

    if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
        Color.emissive = Vector3(color.r, color.g, color.b);

    /* Get diffuse textures */
    ntex = mat->GetTextureCount(aiTextureType_DIFFUSE);
    for (uint32_t i = 0; i < ntex; i++)
    {
        aiString strFile;

        if (mat->GetTexture(aiTextureType_DIFFUSE, i, &strFile, NULL, NULL, NULL, NULL, NULL)
            == AI_SUCCESS)
        {
            uint32_t idx = texMgr->addTextureFromFile(strFile.C_Str());
            if (idx != TEXTURE_INVALID)
                texIdx.diffuse.push_back(idx);
        }
    }

    /* Get normal-map textures */
    ntex = mat->GetTextureCount(aiTextureType_NORMALS);
    for (uint32_t i = 0; i < ntex; i++)
    {
        aiString strFile;

        if (mat->GetTexture(aiTextureType_NORMALS, i, &strFile, NULL, NULL, NULL, NULL, NULL)
            == AI_SUCCESS)
        {
            uint32_t idx = texMgr->addTextureFromFile(strFile.C_Str());
            if (idx != TEXTURE_INVALID)
                texIdx.normal.push_back(idx);
        }
    }

    /* Height maps (bump maps) are also stored as normal maps (to fix) */
    ntex = mat->GetTextureCount(aiTextureType_HEIGHT);
    for (uint32_t i = 0; i < ntex; i++)
    {
        aiString strFile;

        if (mat->GetTexture(aiTextureType_HEIGHT, i, &strFile, NULL, NULL, NULL, NULL, NULL)
            == AI_SUCCESS)
        {
            uint32_t idx = texMgr->addTextureFromFile(strFile.C_Str());
            if (idx != TEXTURE_INVALID)
                texIdx.normal.push_back(idx);
        }
    }

    /* Specular maps */
    ntex = mat->GetTextureCount(aiTextureType_SPECULAR);
    for (uint32_t i = 0; i < ntex; i++)
    {
        aiString strFile;

        if (mat->GetTexture(aiTextureType_SPECULAR, i, &strFile, NULL, NULL, NULL, NULL, NULL)
            == AI_SUCCESS)
        {
            uint32_t idx = texMgr->addTextureFromFile(strFile.C_Str());
            if (idx != TEXTURE_INVALID)
                texIdx.specular.push_back(idx);
        }
    }

    /* Opacity (mask) maps */
    ntex = mat->GetTextureCount(aiTextureType_OPACITY);
    for (uint32_t i = 0; i < ntex; i++)
    {
        aiString strFile;

        if (mat->GetTexture(aiTextureType_OPACITY, i, &strFile, NULL, NULL, NULL, NULL, NULL)
            == AI_SUCCESS)
        {
            uint32_t idx = texMgr->addTextureFromFile(strFile.C_Str());
            if (idx != TEXTURE_INVALID)
                texIdx.mask.push_back(idx);
        }
    }

    /* Just for debug */    
    ntex = mat->GetTextureCount(aiTextureType_SHININESS);
    ntex = mat->GetTextureCount(aiTextureType_EMISSIVE);
    ntex = mat->GetTextureCount(aiTextureType_LIGHTMAP);
    ntex = mat->GetTextureCount(aiTextureType_REFLECTION);
    ntex = mat->GetTextureCount(aiTextureType_UNKNOWN);

    return 0;
}
