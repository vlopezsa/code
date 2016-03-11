#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "LightModel.h"

#include "ModelLoader.h"
#include "ImageLoader.h"

#pragma warning(disable:4244)

#ifndef PI
#define PI			3.14159265358979323846
#endif

using namespace std;

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

    progresCbk = NULL;
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
    lightCoeff = new glVector[nBands2];

    // Transport coefficients
    transCoeff = new glVector**[geometry->nMesh];

    for (int i = 0; i < geometry->nMesh; i++)
    {
        transCoeff[i] = new glVector*[geometry->mesh[i].nVertex];
        for (int j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            transCoeff[i][j] = new glVector[nBands2];
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

/*void CLightModel::castLightFromProbe(glVector *color, Point2D *dir)
{
   // if (dir->x>(PI/16))
    {
        color->r = color->g = color->b = 1.0f;
    }
    else
    {
        color->r = color->g = color->b = 0.0f;
    }
}*/

void CLightModel::castLightFromProbe(glVector *color, glVector *dir)
{
    float d, r;
    Point2D tex;
    int pxCoord[2];
    int pxIdx;

    color->r = color->g = color->b = 0.0f;

    /*if(dir->y > -0.5f && dir->x > -0.50f &&
       dir->y <  0.5f && dir->x <  0.50f)*/
    //if(dir->z > -0.5f && dir->x > -0.50f &&
    //   dir->z <  0.5f && dir->x <  0.50f)
    if(dir->y > 0.0f && dir->x > 0.0f)
    {
            color->r = color->g = color->b = 1.0f;
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

void CLightModel::projectLight(glVector *coeff)
{
    glVector *cf;
    glVector color;

    int nBands2 = nBands * nBands;
    int nSamples = shSampler.getNumberSamples();

    float scale = (4.0*PI) / (float)nSamples;

    cf = coeff;

    for (int i=0; i < nSamples; i++)
    {
        tSphereSample *sample = shSampler.getSample(i);

        castLightFromProbe(&color, &sample->cart);
        //castLightFromProbe(&color, &sample->sphr);
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

bool RayIntersectsTriangle(glVector *p, glVector *d,
    glVector* v0, glVector* v1, glVector* v2)
{
    glVector e1, e2, p1, i1, t1, q1;
    float det = 0.0f;

    e1 = *v1 - *v0;
    e2 = *v2 - *v1;

    p1 = cross(*d, e2);

    float a = dot(e1, p1);

    if (a > -0.00001 && a < 0.00001)
        return false;

    det = 1.0f / a;

    t1 = *d - *v0;

    i1.x = dot(t1, p1)*det;

    if (i1.x < 0.0f || i1.x > 1.0f)
        return false;

    q1 = cross(t1, e1);
    
    i1.y = dot(*d, q1)*det;

    if (i1.y < 0.0f || (i1.x + i1.y) > 1.0f)
        return false;

    i1.z = dot(e2, q1)*det;

    if (i1.z > 0.00001f)
        return true;

    return false;
}
bool CLightModel::Visibility(int vIdx, int mIdx, glVector *dir)
{
    bool visible(true);
    int baset = (vIdx / 3) * 3;

    glVector p = geometry->mesh[mIdx].vertex[vIdx].p;

    for (int i = 0; i < geometry->nMesh; i++)
    {
        for (int j = 0; j < geometry->mesh[i].nIndex; j+=3)
        {
            unsigned int t = geometry->mesh[i].index[j];
            //if ((vIdx != t) && (vIdx!= t+1) && (vIdx!=t+2))
            {
                glVector v0 = geometry->mesh[i].vertex[t+0].p;
                glVector v1 = geometry->mesh[i].vertex[t+1].p;
                glVector v2 = geometry->mesh[i].vertex[t+2].p;
                visible = !RayIntersectsTriangle(&p, dir, &v0, &v1, &v2);
                if (!visible)
                    return false;
            }
        }
    }
    
    return true;
}

void CLightModel::projectShadow(glVector *coeff, int mIdx, int vIdx)
{
    glVector *cf;
    glVector color;

    float costerm = 0.0f;
    float scale = (4.0*PI) / (float)nSamples;

    cf = coeff;

    for (int i = 0; i < nSamples; i++)
    {
        tSphereSample *sample = shSampler.getSample(i);
            
        if (Visibility(vIdx, mIdx, &sample->cart))
        {
            costerm = dot(glVector(geometry->mesh[mIdx].vertex[vIdx].n),
                 glVector(sample->cart));

            if (costerm > 0.0f)
            {
                for (int j = 0; j < nBands2; j++)
                {
                    /*color.r = ((geometry->mesh[mIdx].vertex[vIdx].c >> 16) & 0xFF) / 255.0f;
                      color.g = ((geometry->mesh[mIdx].vertex[vIdx].c >>  8) & 0xFF) / 255.0f;
                      color.b = ( geometry->mesh[mIdx].vertex[vIdx].c        & 0xFF) / 255.0f;*/

                    cf[j] += (sample->sh[j] * costerm);
                }
            }
        }
    }

    for (int i = 0; i < nBands2; i++)
    {
        cf[i] *= scale;
    }
}

#include <thread>
#include <vector>

/*void CLightModel::projectShadowRange(glVector *coeff, int mIdx, int vIdx_Start, int vIdx_End)
{
    int j;

    for (j = vIdx_Start; j < vIdx_End; j++)
    {
        projectShadow(coeff, mIdx, j);
    }
   
}*/

void CLightModel::computeTransferCoefficients()
{
    int i, j;

    float tt = 1.0f / geometry->nMesh;

    for (i = 0; i < geometry->nMesh; i++)
    {
        updateProgress("Transfer Coefficients - Calculating", tt * (float)i * 100.0f);

        /*int split = (int)((float)(geometry->mesh[i].nVertex) / 8);
        vector<thread> t;
        
        for (j = 0; j < 7; j++)
            t.push_back(thread(&CLightModel::projectShadowRange, this, transCoeff[i][split*j], i, split*j, (split*j) + split));

        t.push_back(thread(&CLightModel::projectShadowRange, this, transCoeff[i][split*j], i, split*j, geometry->mesh[i].nVertex));

        for (j = 0; j < 8; j++)
            t[j].join();*/

        
        float tpm = 1.0f / geometry->mesh[i].nVertex*100.0f;
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            projectShadow(transCoeff[i][j], i, j);

            if (i == 0 && j == 0)
                printf("%f %f %f\n", transCoeff[0][0][0].x, transCoeff[0][0][0].y, transCoeff[0][0][0].z);

            if (j % 100 == 0)
            {                
                updateProgress("Transfer Coefficients - Calculating", tpm*(float)j);
            }
        }
    }
}

int CLightModel::computeCoefficients(int samples, int bands, bool force)
{
    shSampler.setNumberSamples(samples, bands);

    updateProgress("Samples", 0.0f);

    shSampler.calculateSamples();

    updateProgress("Samples", 100.0f);

    if (!geometry)
        return -1;

    nSamples = shSampler.getNumberSamples();
    nBands = shSampler.getNumberBands();
    nBands2 = nBands*nBands;

    updateProgress("Memory Allocation", 0.0f);
    allocateMemory();
    updateProgress("Memory Allocation", 100.0f);

    updateProgress("Light Coefficients", 0.0f);
    computeLightCoefficients();
    updateProgress("Light Coefficients", 100.0f);

    if (force == true)
    {
        updateProgress("Transfer Coefficients", 0.0f);
        computeTransferCoefficients();
        printf("%f %f %f\n", transCoeff[0][0][0].x, transCoeff[0][0][0].y, transCoeff[0][0][0].z);
        updateProgress("Transfer Coefficients - Saving", 0.0f);
        saveTransferCoefficients();
        updateProgress("Transfer Coefficients - Saving", 100.0f);
    }
    else if (loadTranferCoefficients()==false)
    {
        computeTransferCoefficients();
        saveTransferCoefficients();
    }

    updateProgress("All Coefficients", 100.0f);
    
    return 0;
}

void CLightModel::evaluatePRT(tPixel3 *p, int mIdx, int vIdx)
{
    glVector r;

    for (int i = 0; i < nBands2; i++)
    {
        r.r += (lightCoeff[i].r * transCoeff[mIdx][vIdx][i].r);
        r.g += (lightCoeff[i].g * transCoeff[mIdx][vIdx][i].g);
        r.b += (lightCoeff[i].b * transCoeff[mIdx][vIdx][i].b);
    }

    /*p->r = transCoeff[mIdx][vIdx][0].r;
    p->g = transCoeff[mIdx][vIdx][0].g;
    p->b = transCoeff[mIdx][vIdx][0].b;*/
    /* p->r = r.x * colorScale.r;
     p->g = r.y * colorScale.g;
     p->b = r.z * colorScale.b;*/

    //r += colorMin;

    p->r = r.x;
    p->g = r.y;
    p->b = r.z;

    /*p->r = transCoeff[mIdx][vIdx][0].r;
    p->g = transCoeff[mIdx][vIdx][0].g;
    p->b = transCoeff[mIdx][vIdx][0].b;*/

    /*p->r = lightCoeff[0].r;
    p->g = lightCoeff[0].g;
    p->b = lightCoeff[0].b;*/

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

    rsize = sizeof(glVector::x) * 3 * nBands2;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            for (k = 0; k < nBands2; k++)
            {
                fread(&transCoeff[i][j][k].d[0], sizeof(glVector::x)*3, 1, f);
            }
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

    rsize = sizeof(glVector::x) * 3 * nBands2;

    for (i = 0; i < geometry->nMesh; i++)
    {
        for (j = 0; j < geometry->mesh[i].nVertex; j++)
        {
            k = 0;
            for (k = 0; k < nBands2 && !ferror(f); k++)
                fwrite(&transCoeff[i][j][k].d[0], sizeof(glVector::x)*3, 1, f);
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

void CLightModel::setProgressCbk(void(*f)(char *))
{
    if (f)
        progresCbk = f;
}

void CLightModel::updateProgress(char *phase, float percent)
{
    if (!progresCbk)
        return;

    stringstream ss;

    ss << phase << " : " << percent << "%";

    progresCbk((char *)ss.str().c_str());
}