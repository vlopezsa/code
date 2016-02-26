#include <stdio.h>
#include <math.h>
#include <string.h>

#include "LightModel.h"

#include "ModelLoader.h"
#include "ImageLoader.h"

#pragma warning(disable:4244)

#ifndef PI
#define PI			3.14159265358979323846
#endif

CLightModel::~CLightModel()
{
    if (lightCoeff)
    {
        delete[] lightCoeff;
    }

    lightCoeff = NULL;

    if (transCoeff)
    {
        delete[] transCoeff;
    }

    transCoeff = NULL;

    /*if (lightProbe.data)
    {
        delete lightProbe.data;
    }*/
}

CLightModel::CLightModel()
{
    geometry = NULL;

    lightProbe.data = NULL;
    lightProbe.w = 0;
    lightProbe.h = 0;

    nBands = 0;

    lightCoeff = NULL;

    transCoeff = NULL;
}

void CLightModel::setNumberBands(int bands)
{
    if (bands == nBands)
        return;

    nBands = (bands < 0) ? 0 : bands;

    if (lightCoeff)
        delete[] lightCoeff;

    if (transCoeff)
        delete[] transCoeff;
}

void CLightModel::setGeometry(tModel *model)
{
    geometry = model;

    if (lightCoeff)
        delete[] lightCoeff;

    lightCoeff = new tLightCoeff[geometry->nMesh];

    for (int i = 0; i < geometry->nMesh; i++)
    {
        lightCoeff[i].nCoeff = geometry->mesh[i].nVertex;
        lightCoeff[i].coeff = new tPixel3*[geometry->mesh[i].nVertex];
    }

    if (transCoeff)
        delete[] transCoeff;

    transCoeff = new tLightCoeff[geometry->nMesh];

    for (int i = 0; i < geometry->nMesh; i++)
    {
        transCoeff[i].nCoeff = geometry->mesh[i].nVertex;
        transCoeff[i].coeff = new tPixel3*[geometry->mesh[i].nVertex];
    }
}

int CLightModel::setLightProbe(unsigned char *data, int width, int height)
{
    if (!data)
        return -1;

    lightProbe.data = data;
    lightProbe.h = height;
    lightProbe.w = width;

    return 0;
}

int CLightModel::setLightProbeFromFile(char *file)
{
    unsigned char *imgData=NULL;
    int w, h;

    imgData = ImageLoader::LoadFile(file, &w, &h);

    if (imgData == NULL)
        return -1;

    return setLightProbe(imgData, w, h);
}

void CLightModel::castLightFromProbe(tPixel3 *color, Point3D *dir)
{
    float d, r;
    Point2D tex;
    int pxCoord[2];
    int pxIdx;

    d = sqrt(dir->x*dir->x + dir->y*dir->y);
    r = (d == 0) ? 0.0f : (1.0f / PI / 2.0f) * acos(dir->z) / d;
    
    tex.x = 0.5f + dir->x * r;
    tex.y = 0.5f + dir->y * r;
    
    pxCoord[0] = tex.x * lightProbe.w;
    pxCoord[1] = tex.y * lightProbe.h;
    
    pxIdx = (pxCoord[1] * lightProbe.w + pxCoord[0]) * 3;

    color->r = lightProbe.data[pxIdx+0];
    color->g = lightProbe.data[pxIdx+1];
    color->b = lightProbe.data[pxIdx+2];
}

void CLightModel::projectLight(tPixel3 **coeff)
{
    tPixel3 *cf;
    tPixel3 color;

    int nBands2 = nBands * nBands;
    int nSamples = shSampler.getNumberSamples();

    float scale = (4.0*PI) / (float)nSamples;

    cf = new tPixel3[nBands*nBands + 1];

    memset(cf, 0, sizeof(tPixel3)*(nBands2 + 1));

    for (int i=0; i < nSamples; i++)
    {
        tSphereSample *sample = shSampler.getSample(i);

        for (int j = 0; j < nBands2; j++)
        {
            castLightFromProbe(&color, &sample->cart);

            cf[j].r += (color.r * sample->sh[j]);
            cf[j].g += (color.g * sample->sh[j]);
            cf[j].b += (color.b * sample->sh[j]);
        }
    }

    for (int i=0; i < nBands2; i++)
    {
        cf[i].r *= scale;
        cf[i].g *= scale;
        cf[i].b *= scale;
    }

    *coeff = cf;
}

void CLightModel::computeLightCoefficients()
{
    int i, j;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            projectLight(&lightCoeff[i].coeff[j]);
        }
    }
}

void cross(float *o, float *v1, float *v2)
{
    o[0] = v1[1] * v2[2] - v1[2] * v2[1];
    o[1] = v1[2] * v2[0] - v1[0] * v2[2];
    o[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

float dot(float *v1, float *v2)
{
    return	v1[0] * v2[0] +
        v1[1] * v2[1] +
        v1[2] * v2[2];
}

bool RayIntersectsTriangle(Point3D *p, Point3D *d,
    Point3D* v0, Point3D* v1, Point3D* v2)
{
    float e1[3] = { v1->x-v0->x, v1->y - v0->y, v1->z - v0->z };
    float e2[3] = { v2->x-v0->x, v2->y - v0->y, v2->z - v0->z };
    float h[3];
    cross(h, (float *)d, e2);
    float a = dot(e1, h);
    if (a > -0.00001f && a < 0.00001f)
        return(false);
    float f = 1.0f / a;
    float s[3] = { p->x - v0->x, p->y - v0->y, p->z - v0->z };
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return(false);
    float q[3];
    cross(q, s, e1);
    float v = f * dot((float *)d, q);
    if (v < 0.0f || u + v > 1.0f)
        return(false);
    float t = dot(e2, q)*f;
    if (t < 0.0f)
        return(false);
    return(true);
}

bool CLightModel::Visibility(int vIdx, int mIdx, Point3D *dir)
{
    bool visible(true);

    Point3D &p = geometry->mesh[mIdx].vertex[vIdx].p;

    for (int i = 0; i < geometry->nMesh; i++)
    {
        for (int j = 0; j < geometry->mesh[i].nIndex; j+=3)
        {
            unsigned int &t = geometry->mesh[i].index[j];
            if ((vIdx != t) && (vIdx!= t+1) && (vIdx != t+2) &&(i!=mIdx))
            {
                Point3D& v0 = geometry->mesh[i].vertex[t+0].p;
                Point3D& v1 = geometry->mesh[i].vertex[t+1].p;
                Point3D& v2 = geometry->mesh[i].vertex[t+2].p;
                visible = !RayIntersectsTriangle(&p, dir, &v0, &v1, &v2);
                if (!visible)
                    break;
            }
        }
    }
    
    return(visible);
}

void CLightModel::projectShadow(tPixel3 **coeff, int mIdx, int vIdx)
{
    tPixel3 *cf;
    tPixel3 color;

    int nBands2 = nBands * nBands;
    int nSamples = shSampler.getNumberSamples();

    float costerm = 0.0f;
    float scale = (4.0*PI) / (float)nSamples;

    cf = new tPixel3[nBands*nBands + 1];

    memset(cf, 0, sizeof(tPixel3)*(nBands2 + 1));

    for (int i = 0; i < nSamples; i++)
    {
        tSphereSample *sample = shSampler.getSample(i);
            
        if (Visibility(vIdx, mIdx, &sample->cart))
        {
            costerm = dot((float *)&geometry->mesh[mIdx].vertex[vIdx].n,
                 (float *)&sample->cart);

            for (int j = 0; j < nBands2; j++)
            {
                color.r = ((geometry->mesh[mIdx].vertex[vIdx].c >> 16) & 0xFF) / 255.0f;
                color.g = ((geometry->mesh[mIdx].vertex[vIdx].c >>  8) & 0xFF) / 255.0f;
                color.b = ( geometry->mesh[mIdx].vertex[vIdx].c        & 0xFF) / 255.0f;

                cf[j].r += (color.r * sample->sh[j]) * costerm;
                cf[j].g += (color.g * sample->sh[j]) * costerm;
                cf[j].b += (color.b * sample->sh[j]) * costerm;
            }
        }
    }

    for (int i = 0; i < nBands2; i++)
    {
        cf[i].r *= scale;
        cf[i].g *= scale;
        cf[i].b *= scale;
    }

    *coeff = cf;
}


void CLightModel::computeTransferCoefficients()
{
    int i, j;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            projectShadow(&transCoeff[i].coeff[j], i, j);
        }
    }
}

int CLightModel::computeCoefficients()
{
    shSampler.setNumberSamples(225, nBands);

    shSampler.calculateSamples();

    computeLightCoefficients();

    computeTransferCoefficients();

    return 0;
}

void CLightModel::evaluatePRT(tPixel3 *p, int mIdx, int vIdx)
{
    int nBands2 = nBands * nBands;

    memset(p, 0, sizeof(tPixel3));

    for (int i = 0; i < nBands2; i++)
    {
        p->r += (lightCoeff[mIdx].coeff[0][i].r *
                 transCoeff[mIdx].coeff[vIdx][i].r);
        p->g += (lightCoeff[mIdx].coeff[0][i].g *
                 transCoeff[mIdx].coeff[vIdx][i].g);
        p->b += (lightCoeff[mIdx].coeff[0][i].b *
                 transCoeff[mIdx].coeff[vIdx][i].b);
    }
}