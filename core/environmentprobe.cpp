#define _USE_MATH_DEFINES
#include <math.h>

#include "environmentprobe.h"


EnvironmentProbe::EnvironmentProbe(uint32_t numDivisions)
    : EnvironmentMap()
{
    if (numDivisions == 0)
        numDivisions = 2;
    createSphere(numDivisions);

    texMgr = NULL;
}

EnvironmentProbe::EnvironmentProbe()
    : EnvironmentMap()
{
    createSphere(1024);

    texMgr = NULL;
}

void EnvironmentProbe::createSphere(uint32_t divisions)
{
    float const R = 1.0f / (float)(divisions - 1);
    float const S = 1.0f / (float)(divisions - 1);
    uint32_t r, s, i=0;

    mesh.clear();

    mesh.resize(1);

    mesh[0].setNumVertices(divisions * divisions * 3);

    for (r = 0; r < divisions; r++)
    {
        for (s = 0; s < divisions; s++)
        {
            float phi = (float)(2 * M_PI * s * S);
            float theta = (float)(M_PI * r * R);

            Vertex v;

            v.position.x = (float)(cos(phi) * sin(theta));
            v.position.y = (float)sin(-M_PI_2 + theta);
            v.position.z = (float)(sin(phi) * sin(theta));

            v.normal.x = -v.position.x;
            v.normal.y = -v.position.y;
            v.normal.z = -v.position.z;

            v.normal.normalize();

            v.tex = getUVCoord(v.position);

            mesh[0].addVertex(i, v);

            i++;
        }
    }

    mesh[0].index.resize(divisions * divisions * 4);
    std::vector<uint32_t>::iterator idx = mesh[0].index.begin();
    for (r = 0; r < divisions - 1; r++)
    {
        for (s = 0; s < divisions - 1; s++) {
            *idx++ = r * divisions + s;
            *idx++ = r * divisions + (s + 1);
            *idx++ = (r + 1) * divisions + (s + 1);
            *idx++ = (r + 1) * divisions + s;
        }
    }
}

Vector2 EnvironmentProbe::getUVCoord(Vector3 dir)
{
    float d = sqrt(dir.x*dir.x + dir.y*dir.y);
    float r = (d == 0) ? 0.0f : (1.0f / M_PI / 2.0f) * acos(dir.z) / d;
    Vector2 uv;


    uv.x = 0.5f + dir.x * r;
    uv.y = 0.5f + dir.y * r;

    return uv;
}

bool EnvironmentProbe::setLightProbe(TextureManager *extTexMgr, char *file)
{
    material.clear();

    material.resize(1);

    uint32_t tidx = extTexMgr->addTextureFromFile(file);

    material[0].Color.diffuse = Vector3();

    if (tidx != TEXTURE_INVALID)
        material[0].texIdx.diffuse.push_back(tidx);

    mesh[0].materialIdx = 0;

    this->texMgr = extTexMgr;

    return true;
}

Vector3 EnvironmentProbe::getSampleColor(Vector3 dir)
{
    return Vector3();
}

Vector3 EnvironmentProbe::getSampleDir(const Vector3 &cartDir)
{
    Vector3 color;
    Vector2 uv = getUVCoord(cartDir);

    Material *mat = &material[mesh[0].materialIdx];
    
    Image *img = texMgr->getTextureImage(mat->texIdx.diffuse[0]);

    uint32_t x, y;
    
    x = (uint32_t)(uv.x*(float)img->Width);
    y = (uint32_t)(uv.y*(float)img->Height);

    uint32_t offset = y*img->Width + x;

    offset *= 3;

    color.r = (float)img->Data[offset + 0] / 255.0f;
    color.g = (float)img->Data[offset + 1] / 255.0f;
    color.b = (float)img->Data[offset + 2] / 255.0f;

    return color;
}

Vector3 EnvironmentProbe::getSampleDir(const Vector2 &sphrDir)
{
    return Vector3();
}

