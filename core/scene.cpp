#include <assimp\scene.h>
#include <assimp\cimport.h>
#include <assimp\config.h>
#include <assimp\postprocess.h>

#include "scene.h"

void Scene::__release()
{
    mesh.clear();
    material.clear();
}

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
    __release();
}

bool Scene::loadFromFile(char * strFile)
{
    const aiScene *pScene;
    aiPropertyStore *pStore;

    pStore = aiCreatePropertyStore();

    aiSetImportPropertyInteger(pStore, AI_CONFIG_PP_FD_REMOVE, 1);
    aiSetImportPropertyInteger(pStore, AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

    pScene = aiImportFileExWithProperties(strFile,
        //aiProcessPreset_TargetRealtime_Quality |
        //aiProcess_JoinIdenticalVertices |
        //aiProcess_PreTransformVertices |
        aiProcess_Triangulate |
        //aiProcess_GenSmoothNormals |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData |
        //aiProcess_RemoveRedundantMaterials |
        //aiProcess_GenUVCoords |
        aiProcess_SortByPType |
        0,
        NULL,
        pStore); 

    if (!pScene)
    {
        aiReleasePropertyStore(pStore);
        return false;
    }

    __release();

    strName = std::string(strFile);

    /* Loading meshes*/

    mesh.resize(pScene->mNumMeshes);
    
    for (unsigned int i=0; i < mesh.size(); i++)
    {
        mesh[i].importAIMesh(pScene->mMeshes[i]);
    }

    /* Loading materials */
    material.resize(pScene->mNumMaterials);
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
    {
        material[i].importAIMaterial(&texture, pScene->mMaterials[i]);
    }

    aiReleaseImport(pScene);
    aiReleasePropertyStore(pStore);

    return false;
}
