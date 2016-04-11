#pragma once

#include <unordered_map>

#include "scene.h"
#include "graphics.h"

class Render
{
private:
    uint32_t _addGPUTexture(uint32_t texIdx);

    Scene *scene;
    std::unordered_map<uint32_t, uint32_t> gpuTex;

    bool   preComputedEnvLight;

public:
    Render();
    ~Render();

    void clear(Vector3 color);
    void swapBuffers();

    void updateCamera(Camera *cam);

    void setupMaterial(Material *mat);
    void renderMesh(Mesh *m);
    void renderScene(Scene *s);

    void usePreComputedEnvLight(bool enable = false);
};

