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

int CLightModel::setLightProbe(uint8_t *data, int width, int height)
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
    uint8_t *imgData=NULL;
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
    if(dir->y > 0.0f && dir->x > 0.0f && dir->z > 0.0f)
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
    e2 = *v2 - *v0;

    p1 = cross(*d, e2);

    float a = e1*p1;

    if (a > -0.00001 && a < 0.00001)
        return false;

    det = 1.0f / a;

    t1 = *p - *v0;

    i1.x = dot(t1, p1)*det;

    if (i1.x < 0.0f || i1.x > 1.0f)
        return false;

    q1 = cross(t1, e1);
    
    i1.y = dot(*d, q1)*det;

    if (i1.y < 0.0f || (i1.x + i1.y) > 1.0f)
        return false;

    i1.z = dot(e2, q1)*det;

    if (i1.z > 0.000001f)
        return true;

    return false;
}


bool CLightModel::Visibility(int vIdx, int mIdx, glVector *dir)
{
    bool visible(true);
    int baset = vIdx - (vIdx%3);

    glVector p = geometry->mesh[mIdx].vertex[vIdx].p;

   p += (glVector(geometry->mesh[mIdx].vertex[vIdx].n)*0.02f);
         
   // glVector ndir = p + geometry->mesh[mIdx].vertex[vIdx].n;

    for (int i = 0; i < geometry->nMesh; i++)
    {
        for (int j = 0; j < geometry->mesh[i].nIndex; j+=3)
        {
            unsigned int t = geometry->mesh[i].index[j];
            
            if ((baset != t) && (baset != t + 1) && (baset != t + 2))
            {
                glVector v0 = geometry->mesh[i].vertex[t+0].p;
                glVector v1 = geometry->mesh[i].vertex[t+1].p;
                glVector v2 = geometry->mesh[i].vertex[t+2].p;
                visible = !RayIntersectsTriangle(&p, dir,
                    &v0, &v1, &v2);
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
                    cf[j] += (sample->sh[j] * costerm);
     //               cf[j] += 1.0f;
                }    
            }
        }
    }

   // scale = 1.0f / (float)nSamples;

    for (int i = 0; i < nBands2; i++)
    {
        cf[i] *= scale;
    }
}

#include <thread>
#include <vector>
#include <mutex>

int cntProgress, cntTotal;
mutex mtxProgress;


void CLightModel::projectShadowRange(glVector **coeff, int mIdx,
    int vIdx_Start, int vIdx_End)
{
    int i, j;

    for (i=0, j = vIdx_Start; j < vIdx_End; i++,j++)
    {
        projectShadow(coeff[j], mIdx, j);

        if (i == 100)
        {
            mtxProgress.lock();
            cntProgress += i;
            printf("Transfer Coefficients - Calculating: %d / %d\r",
                cntProgress, cntTotal);
            mtxProgress.unlock();

            i = 0;
        }
    }
   
}

void CLightModel::computeTransferCoefficients()
{
    int i, j;

    float tt = 1.0f / geometry->nMesh;

    cntProgress = 0;
    cntTotal = 0;

    int nT = 16;

    for (i = 0; i < geometry->nMesh; i++)
        cntTotal += geometry->mesh[i].nVertex;

    printf("Total Vertices to compute: %d\n", cntTotal);

    for (i = 0; i < geometry->nMesh; i++)
    {
        updateProgress("Transfer Coefficients - Calculating", tt * (float)i * 100.0f);

     //   projectShadowRange(transCoeff[i], i, 0, geometry->mesh[i].nVertex);

        int split = (int)((float)(geometry->mesh[i].nVertex) / nT);
        vector<thread> t;
        
        for (j = 0; j < nT-1; j++)
            t.push_back(thread(&CLightModel::projectShadowRange, this,
                transCoeff[i], i, split*j, (split*j) + split));

        t.push_back(thread(&CLightModel::projectShadowRange, this,
            transCoeff[i], i, split*j, geometry->mesh[i].nVertex));

        for (j = 0; j < nT; j++)
            t[j].join();
    }

    printf("\n");
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
        /*r.r += (transCoeff[mIdx][vIdx][i].r);
        r.g += (transCoeff[mIdx][vIdx][i].g);
        r.b += (transCoeff[mIdx][vIdx][i].b);*/
    }
    p->r = r.x;
    p->g = r.y;
    p->b = r.z;
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