#pragma once
#include "environmentmap.h"
class EnvironmentCubeMap :
    public EnvironmentMap
{
    void createCube();

public:
    EnvironmentCubeMap();
    ~EnvironmentCubeMap();

    Vector3 getSampleDir(const Vector3 &cartDir);
    Vector3 getSampleDir(const Vector2 &sphrDir);

};

