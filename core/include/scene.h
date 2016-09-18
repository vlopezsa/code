#pragma once

#include <vector>
#include <string>

#include "graphics.h"
#include "camera.h"
#include "environmentmap.h"
#include "mesh.h"
#include "material.h"
#include "texturemgr.h"
#include "BVH.h"

class Scene {
private:
    void __release();

    void __calculteTriangleTotal();

public:
    std::string           strName;
    std::vector<Mesh>     mesh;
    std::vector<Material> material;
    std::vector<Object *> trilist;
    TextureManager        texture;
    Camera                camera;
    EnvironmentMap       *envMap;
    BVH                  *bvh;

    uint32_t              triTotal;

public:
    Scene();
    Scene(char *strFile);
    ~Scene();

    bool loadFromFile(char *strFile);

    void setNumMeshes(uint32_t nMesh) { mesh.resize(nMesh); }
    void setNumMaterials(uint32_t nMat) { material.resize(nMat); }

    uint32_t numMeshes() { return (uint32_t)mesh.size(); }
    uint32_t numMaterials() { return (uint32_t)material.size(); }
    uint32_t numTriangles() { return triTotal; }

    void buildBVH();
};
