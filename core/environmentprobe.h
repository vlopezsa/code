#pragma once

#include "texturemgr.h"
#include "environmentmap.h"

class EnvironmentProbe :
    public EnvironmentMap
{
private:
    Vector2 getUVCoord(Vector3 dir);
    Vector3 getSampleColor(Vector3 dir);

    void  createSphere(uint32_t divisions);

public:
    EnvironmentProbe();
    EnvironmentProbe(uint32_t numDivisions);

    Vector3 getSampleDir(const Vector3 &cartDir);
    Vector3 getSampleDir(const Vector2 &sphrDir);

    bool setLightProbe(TextureManager *texMgr, char *file);
};
