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
    finish();
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

void CLightModel::setGeometry(tModel *model)
{
    geometry = model;
}

void CLightModel::finish()
{
    deallocateMemory();
    geometry = NULL;
}

void CLightModel::allocateMemory()
{
    deallocateMemory();

    if (!geometry)
        return;

    // Light SH Functions
    lightCoeff = new tPixel3[nBands2];

    // Transport coefficients
    transCoeff = new tPixel3**[geometry->nMesh];

    for (int i = 0; i < geometry->nMesh; i++)
    {
        transCoeff[i] = new tPixel3*[geometry->mesh[i].nVertex];
        for (int j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            transCoeff[i][j] = new tPixel3[nBands2];
        }
    }
}

void CLightModel::deallocateMemory()
{
    if (lightCoeff)
    {
        delete[] lightCoeff;
        lightCoeff = NULL;
    }

    if (transCoeff)
    {
        for (int i = 0; i < geometry->nMesh; i++)
        {
            for (int j = 0; j < geometry->mesh[i].nVertex; j++)
            {
                delete[] transCoeff[i][j];
            }
            delete[] transCoeff[i];
        }
        delete[] transCoeff;
        transCoeff = NULL;
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

    if (dir->z > 0.0f && dir->x>0.0f)
    {
        color->r = color->g = color->b = 1.0f;
    }
    else
    {
        color->r = color->g = color->b = 0.0f;
    }

    return;
        

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

void CLightModel::projectLight(tPixel3 *coeff)
{
    tPixel3 *cf;
    tPixel3 color;

    int nBands2 = nBands * nBands;
    int nSamples = shSampler.getNumberSamples();

    float scale = (4.0*PI) / (float)nSamples;

    cf = coeff;

    memset(cf, 0, sizeof(tPixel3)*(nBands2));

    for (int i=0; i < nSamples; i++)
    {
        tSphereSample *sample = shSampler.getSample(i);

        castLightFromProbe(&color, &sample->cart);
        for (int j = 0; j < nBands2; j++)
        {
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
}

void CLightModel::computeLightCoefficients()
{
    projectLight(lightCoeff);
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

void CLightModel::projectShadow(tPixel3 *coeff, int mIdx, int vIdx)
{
    tPixel3 *cf;
    tPixel3 color;

    float costerm = 0.0f;
    float scale = (4.0*PI) / (float)nSamples;

    cf = coeff;

    memset(cf, 0, sizeof(tPixel3)*(nBands2));

    for (int i = 0; i < nSamples; i++)
    {
        tSphereSample *sample = shSampler.getSample(i);
            
        if (Visibility(vIdx, mIdx, &sample->cart))
        {
            costerm = dot((float *)&geometry->mesh[mIdx].vertex[vIdx].n,
                 (float *)&sample->cart);

            if(costerm>0.0f)
                for (int j = 0; j < nBands2; j++)
                {
                    color.r = ((geometry->mesh[mIdx].vertex[vIdx].c >> 16) & 0xFF) / 255.0f;
                    color.g = ((geometry->mesh[mIdx].vertex[vIdx].c >>  8) & 0xFF) / 255.0f;
                    color.b = ( geometry->mesh[mIdx].vertex[vIdx].c        & 0xFF) / 255.0f;

                    /*cf[j].r += (color.r * sample->sh[j]) * costerm;
                    cf[j].g += (color.g * sample->sh[j]) * costerm;
                    cf[j].b += (color.b * sample->sh[j]) * costerm;*/

                    cf[j].r += sample->sh[j] * costerm;
                    cf[j].g += sample->sh[j] * costerm;
                    cf[j].b += sample->sh[j] * costerm;
                }
        }
    }

    for (int i = 0; i < nBands2; i++)
    {
        cf[i].r *= scale;
        cf[i].g *= scale;
        cf[i].b *= scale;
    }
}


void CLightModel::computeTransferCoefficients()
{
    int i, j;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            projectShadow(transCoeff[i][j], i, j);
        }
    }
}

int CLightModel::computeCoefficients(int samples, int bands, bool force)
{
    shSampler.setNumberSamples(samples, bands);
    shSampler.calculateSamples();

    if (!geometry)
        return -1;

    nSamples = shSampler.getNumberSamples();
    nBands = shSampler.getNumberBands();
    nBands2 = nBands*nBands;

    allocateMemory();

    computeLightCoefficients();

    if (force == true)
    {
        computeTransferCoefficients();
        saveTransferCoefficients();
    }
    else if (loadTranferCoefficients()==false)
    {
        computeTransferCoefficients();
        saveTransferCoefficients();
    }

    return 0;
}

void CLightModel::evaluatePRT(tPixel3 *p, int mIdx, int vIdx)
{
    memset(p, 0, sizeof(tPixel3));

    for (int i = 0; i < nBands2; i++)
    {
        p->r += (lightCoeff[i].r *
                 transCoeff[mIdx][vIdx][i].r);
        p->g += (lightCoeff[i].g *
                 transCoeff[mIdx][vIdx][i].g);
        p->b += (lightCoeff[i].b *
                 transCoeff[mIdx][vIdx][i].b);
    }
}

bool CLightModel::loadTranferCoefficients()
{
    FILE *f;

    char fileName[1040] = { 0 };

    int fband, fsample;

    strcat(fileName, geometry->file);
    strcat(fileName, ".tcf");

    f = fopen(fileName, "rb");
    if (!f)
        return false;

    // Read number of bands and number of samples
    fsample = 0;
    fband = 0;
    fread(&fsample, sizeof(int), 1, f);
    fread(&fband, sizeof(int), 1, f);

    // number of bands and samples must be the same
    if (fsample != shSampler.getNumberSamples() ||
        fband != shSampler.getNumberBands())
    {
        fclose(f);
        return false;
    }

    int i, j;

    size_t k, rsize;

    rsize = sizeof(tPixel3) * nBands2;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            //k = 0;
            //while (!ferror(f) && !feof(f) && k < rsize)
            //{
                //k += fread(&(transCoeff[i].coeff[j])+k, sizeof(tPixel3), nBands2, f);

                for (k = 0; k < nBands2; k++)
                    fread(&transCoeff[i][j][k], sizeof(tPixel3), 1, f);
            //}
        }
    }

    if (i < geometry->nMesh)
    {
        fclose(f);
        return false;
    }

    fclose(f);

    return true;
}

bool CLightModel::saveTransferCoefficients()
{
    FILE *f;

    char fileName[1040] = { 0 };

    int fband, fsample;

    strcat(fileName, geometry->file);
    strcat(fileName, ".tcf");

    f = fopen(fileName, "wb");
    if (!f)
        return false;

    fband = shSampler.getNumberBands();
    fsample = shSampler.getNumberSamples();

    fwrite(&fsample, sizeof(int), 1, f);
    fwrite(&fband, sizeof(int), 1, f);

    int i, j;

    size_t k, rsize;

    rsize = sizeof(tPixel3) * nBands2;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            k = 0;
            for (k = 0; k < nBands2 && !ferror(f); k++)
                fwrite(&transCoeff[i][j][k], sizeof(tPixel3), 1, f);
            //while (!ferror(f) && k < rsize)
            //{
                //k += fwrite(&transCoeff[i].coeff[j]+k, sizeof(tPixel3), nBands2, f);
            //}
        }
    }

    if (i < geometry->nMesh)
    {
        fclose(f);
        return false;
    }


    fclose(f);

    return true;
}