#pragma once

#include <vector>
#include <string>

#include "graphics.h"
#include "mesh.h"

class Scene {
public:
    std::string  strName;
    std::vector<Mesh> mesh;

public:
    Scene();
    Scene(char *strFile);
    ~Scene();

    bool loadFromFile(char *strFile);

    size_t numMeshes() { return mesh.size(); }
};
