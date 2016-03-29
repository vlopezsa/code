#include <assimp\scene.h>
#include <assimp\cimport.h>
#include <assimp\config.h>
#include <assimp\postprocess.h>

#include "scene.h"

Scene::Scene()
{
}

Scene::Scene(char * strFile)
{
    try {
        loadFromFile(strFile);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

Scene::~Scene()
{
}

bool Scene::loadFromFile(char * strFile)
{
    const aiScene *pScene;
    aiPropertyStore *pStore;

    pStore = aiCreatePropertyStore();

    aiSetImportPropertyInteger(pStore, AI_CONFIG_PP_FD_REMOVE, 1);
    aiSetImportPropertyInteger(pStore, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    pScene = aiImportFileExWithProperties(strFile,
        aiProcessPreset_TargetRealtime_Quality |
        0,
        NULL,
        pStore); 

    if (!pScene)
    {
        aiReleasePropertyStore(pStore);
        return false;
    }

    strName = std::string(strFile);

    mesh.resize(pScene->mNumMeshes);
    
    for (unsigned int i=0; i < mesh.size(); i++)
    {
        mesh[i].importAIMesh(pScene->mMeshes[i]);
    }

    aiReleaseImport(pScene);
    aiReleasePropertyStore(pStore);

    return false;
}
