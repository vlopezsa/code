#pragma once

#include "mesh.h"

class Light
{
public:
    Light() {}
    ~Light() {}

    virtual float getIntensityAt(Vertex &v) { return 1.0f; }
};