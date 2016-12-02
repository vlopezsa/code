#include <assimp\scene.h>
#include <assimp\cimport.h>
#include <assimp\config.h>
#include <assimp\postprocess.h>

#include <vector>

#include "scene.h"
#include "bvh.h"
#include "triangleobj.h"

void Scene::__release()
{
    mesh.clear();
    material.clear();

    if (envMap)
    {
        delete envMap;
    }
    envMap = NULL;

    if (bvh)
    {
        delete bvh;
    }

    bvh = NULL;
}

void Scene::__calculteTriangleTotal()
{
    triTotal = 0;

    for (int i = 0; i < mesh.size(); i++)
        triTotal += (uint32_t)mesh[i].numTriangles();
}

Scene::Scene()
{
    triTotal = 0;
    envMap = NULL;
}

Scene::Scene(char * strFile)
{
    triTotal = 0;
    envMap = NULL;

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
        aiProcessPreset_TargetRealtime_Quality |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        //aiProcess_Triangulate |
        // aiProcess_GenNormals |
        // aiProcess_GenSmoothNormals |
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

    //__release();
    strName = std::string(strFile);

    /* Loading meshes*/

    mesh.resize(pScene->mNumMeshes);
    
    /* Loading materials */
    material.resize(pScene->mNumMaterials);
    for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
    {
        material[i].importAIMaterial(&texture, pScene->mMaterials[i]);
    }

    for (uint32_t i=0; i < mesh.size(); i++)
    {
        mesh[i].importAIMesh(pScene->mMeshes[i]);
    }

    aiReleaseImport(pScene);
    aiReleasePropertyStore(pStore);

    __calculteTriangleTotal();

    return true;
}

void Scene::buildBVH()
{
    if (bvh)
        delete bvh;

    
    trilist.clear();
    trilist.reserve(triTotal);

    for (uint32_t i = 0; i < mesh.size(); i++)
    {
        for (uint32_t j = 0; j < mesh[i].numTriangles(); j++)
        {
            trilist.push_back(new TriangleObj(&mesh[i], i, j));
        }
    }

    bvh = new BVH(&trilist);
}