#pragma once

#include <vector>
#include <string>

#include "graphics.h"
#include "mesh.h"
#include "material.h"
#include "texturemgr.h"
#include "vector4.h"

class EnvironmentMap
{
public:
    std::string           strName;
    std::vector<Mesh>     mesh;
    std::vector<Material> material;

    TextureManager       *texMgr;

	Vector3				  m_DefaultColor;

public:
	EnvironmentMap() { texMgr = NULL; m_DefaultColor = Vector3(1.0f); }
    ~EnvironmentMap()
    {
        mesh.clear();
    }

    virtual Vector3 getSampleDir(const Vector3 &cartCoord) { return Vector3();  }
    virtual Vector3 getSampleDir(const Vector2 &sphereCoord) { return Vector3(); }
};