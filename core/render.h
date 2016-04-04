#pragma once

#include <unordered_map>

#include "scene.h"
#include "graphics.h"

class Render
{
private:
    unsigned int _addGPUTexture(unsigned int texIdx);

    Scene *scene;
    std::unordered_map<unsigned int, unsigned int> gpuTex;


public:
    Render();
    ~Render();

    void clear(Vector3 color);
    void swapBuffers();

    void updateCamera(Camera *cam);

    void setupMaterial(Material *mat);
    void renderMesh(Mesh *m);
    void renderScene(Scene *s);
};

