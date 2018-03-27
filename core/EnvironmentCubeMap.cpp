#include "EnvironmentCubeMap.h"



EnvironmentCubeMap::EnvironmentCubeMap()
{
}


EnvironmentCubeMap::~EnvironmentCubeMap()
{
}

void EnvironmentCubeMap::createCube()
{
    mesh.clear();

    mesh.resize(6);


}

Vector3 EnvironmentCubeMap::getSampleDir(const Vector3 &cartDir)
{
    Vector3 color;

    return color;
}

Vector3 EnvironmentCubeMap::getSampleDir(const Vector2 &sphrDir)
{
    Vector3 color;

    return color;
}