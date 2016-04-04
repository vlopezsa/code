#pragma once

#include <vector>
#include <string>

#include "graphics.h"
#include "mesh.h"
#include "material.h"
#include "texturemgr.h"
#include "camera.h"

class Scene {
private:
    void __release();

public:
    std::string           strName;
    std::vector<Mesh>     mesh;
    std::vector<Material> material;
    TextureManager        texture;
    Camera                camera;

public:
    Scene();
    Scene(char *strFile);
    ~Scene();

    bool loadFromFile(char *strFile);

    size_t numMeshes() { return mesh.size(); }
    size_t numMaterials() { return material.size(); }
};
