#define _USE_MATH_DEFINES
#include <math.h>

#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>

#include "prt.h"
#include "BVH.h"
#include "mesh.h"
#include "triangleobj.h"

PRT::PRT()
{
    sh = NULL;
    sampler = NULL;

    nBands = 2;
}

PRT::PRT(Sampler * s)
{
    sh = NULL;
    sampler = s;

    nBands = 2;

    sh = new SHStandard(s, nBands);
}

PRT::~PRT()
{
    if (sh)
    {
        delete sh;
        sh = NULL;
    }

    if (sampler)
    {
        delete sampler;
        sampler = NULL;
    }
}

void PRT::setSampler(Sampler * s)
{
    sh = new SHStandard(s, nBands);
    sh->calculateCoefficients();
    sh->calculateScaleFactors();
    sampler = s;
}

#define myclamp(x, min, max) {\
    if (x > max)\
        x = max;\
    else if (x < min)\
        x = min;\
}

Vector3 PRT::getIntensityAt(Vertex & v, bool clamp)
{
    Vector3 intensity;

    if (!sh)
        intensity;

    for (uint32_t i = 0; i < sh->numBaseCoeff; i++)
    {
        intensity.x += lightCoeff[i].x * v.sh_coeff[i];
        intensity.y += lightCoeff[i].y * v.sh_coeff[i];
        intensity.z += lightCoeff[i].z * v.sh_coeff[i];
    }

    if (clamp)
    {
        myclamp(intensity.x, 0.0f, 1.0f);
        myclamp(intensity.y, 0.0f, 1.0f);
        myclamp(intensity.z, 0.0f, 1.0f);
    }

    return intensity;
}

void PRT::computeLightIntensityAtVertices()
{
    Vector3 intensity;

    for (uint32_t i = 0; i < scene->numMeshes(); i++)
    {
        Mesh *m = &scene->mesh[i];
        Material *mat = &scene->material[m->materialIdx];

        for (uint32_t j = 0; j < m->numVertices(); j++)
        {
            intensity = getIntensityAt(m->vertex[j]);

            m->vertex[j].diffuse.r = intensity.r*mat->Color.diffuse.r;
            m->vertex[j].diffuse.g = intensity.g*mat->Color.diffuse.g;
            m->vertex[j].diffuse.b = intensity.b*mat->Color.diffuse.b;
        }
    }
}

int PRT::preComputeLight()
{
    if (!sampler)
        return -1;

    if (!sh)
        return -1;

    lightCoeff.clear();
    lightCoeff.resize(sh->numBaseCoeff);

    float intensity;

    for (uint32_t i = 0; i < sampler->numSamples; i++)
    {
        /* for testing purposing, using directional light */
        if (sampler->Samples[i].Spherical.theta <  0.5f &&
            sampler->Samples[i].Spherical.theta > -0.5f
            )
       /* if(
            sampler->Samples[i].Cartesian.y > 0.0f &&
            sampler->Samples[i].Cartesian.x > -0.2f
          )*/
            intensity = 1.0f;
        else
            intensity = 0.0f;

        /* projecting light in each band per sampling */
        for (uint32_t j = 0; j < sh->numBaseCoeff; j++)
        {
            lightCoeff[j] += intensity * sh->Coefficient[i][j];
        }
    }

    /* normalize light coeff */
    sh->scaleFunctionCoeff(lightCoeff);

    return 0;
}

void PRT::preComputeVertexCoeff(Vertex & v, uint32_t &iMesh, uint32_t &iTrian)
{
    v.sh_coeff.clear();
    v.sh_coeff.resize(sh->numBaseCoeff);

    float costerm;

    IntersectionInfo intInf; //not used

    bool occluded = false;

    Vector3 posOffset = v.position + (v.normal*0.02f);

    /* project vertice in all bands per sample */
    for (uint32_t i = 0; i < sampler->numSamples; i++)
    {
        Ray ray(posOffset,
            sampler->Samples[i].Cartesian
            );

         /* this will generate shadows */
        occluded = scene->bvh->getIntersection(ray, &intInf, true);

        if (occluded)
        {
            TriangleObj *to = (TriangleObj *)intInf.object;
            /* make sure we are not hitting the same triangle */
            if (to->meshId == iMesh && to->triId == iTrian)
                occluded = false;
        }

        if (!occluded)
        {
            costerm = dot(v.normal, sampler->Samples[i].Cartesian);

            if (costerm > 0.0f)
            {
                for (uint32_t j = 0; j < sh->numBaseCoeff; j++)
                {
                    v.sh_coeff[j] += costerm * sh->Coefficient[i][j];
                }
            }
        }
    }

    /* normalize coefficients */
    sh->scaleFunctionCoeff(v.sh_coeff);
}

bool PRT::loadCoeffFromFile()
{
    FILE *f;
    std::stringstream ss;

    ss << scene->strName << ".tcf";

    f = fopen(ss.str().c_str(), "rb");
    if (!f)
        return false;

    uint32_t ns, nb;

    // Check that the file was created with 
    // the same number of samples and bands
    fread(&ns, sizeof(uint32_t), 1, f);
    fread(&nb, sizeof(uint32_t), 1, f);

    if (ns != sampler->numSamples || nb != sh->numBands)
    {
        fclose(f);
        return false;
    }

    // Load the coeff for the vertices
    for (uint32_t i = 0; i < scene->numMeshes(); i++)
    {
        Mesh *m = &scene->mesh[i];
        for (uint32_t j = 0; j < m->numVertices(); j++)
        {
            m->vertex[j].sh_coeff.clear();
            m->vertex[j].sh_coeff.resize(sh->numBaseCoeff);

            for (uint32_t k = 0; k < sh->numBaseCoeff; k++)
            {
                fread(&m->vertex[j].sh_coeff[k], sizeof(float), 1, f);
            }
        }
    }

    fclose(f);

    return true;
}

bool PRT::saveCoeffToFile()
{
    FILE *f;
    std::stringstream ss;

    ss << scene->strName << ".tcf";

    f = fopen(ss.str().c_str(), "wb");
    if (!f)
        return false;

    // Check that the file was created with 
    // the same number of samples and bands
    fwrite(&sampler->numSamples, sizeof(uint32_t), 1, f);
    fwrite(&sh->numBands, sizeof(uint32_t), 1, f);

    // Load the coeff for the vertices
    for (uint32_t i = 0; i < scene->numMeshes(); i++)
    {
        Mesh *m = &scene->mesh[i];
        for (uint32_t j = 0; j < m->numVertices(); j++)
        {
            if (m->vertex[j].sh_coeff.size() != sh->numBaseCoeff)
            {
                std::cerr << "[ ERROR ] In Mesh " << i << " Vertex " << j;
                std::cerr << " was not processed" << std::endl;

                m->vertex[j].sh_coeff.resize(sh->numBaseCoeff);
            }
            else
            {
                for (uint32_t k = 0; k < sh->numBaseCoeff; k++)
                {
                    fwrite(&m->vertex[j].sh_coeff[k], sizeof(float), 1, f);
                }
            }
        }
    }

    fclose(f);

    return true;
}

void PRT::threadWorker(Mesh * m, uint32_t vIdx_Start, uint32_t vIdx_End)
{
    for (uint32_t j = vIdx_Start, i=0; j < m->numTriangles() && j<vIdx_End; j++, i++)
    {
        uint32_t &iv1 = m->triangle[j].v1;
        uint32_t &iv2 = m->triangle[j].v2;
        uint32_t &iv3 = m->triangle[j].v3;

        bool bC = false; // compute vertex;

        threadInfo.mtxVertex.lock();
        if (!threadInfo.vtxVisited[iv1])
        {
            threadInfo.vtxVisited[iv1] = true;
            bC = true;
        }
        else
            bC = false;
        threadInfo.mtxVertex.unlock();

        if(bC)
            preComputeVertexCoeff(m->vertex[iv1], i, j);

        threadInfo.mtxVertex.lock();
        if (!threadInfo.vtxVisited[iv2])
        {
            threadInfo.vtxVisited[iv2] = true;
            bC = true;
        }
        else
            bC = false;
        threadInfo.mtxVertex.unlock();
        
        if (bC)
            preComputeVertexCoeff(m->vertex[iv2], i, j);
        
        threadInfo.mtxVertex.lock();
        if (!threadInfo.vtxVisited[iv3])
        {
            threadInfo.vtxVisited[iv3] = true;
            bC = true;
        }
        else
            bC = false;
        threadInfo.mtxVertex.unlock();

        if (bC)
            preComputeVertexCoeff(m->vertex[iv3], i, j);

        if (i == 100)
        {
            threadInfo.mtxProgress.lock();
            threadInfo.cntProcess += i;
            printf("\t\tTransfer Coefficients - Calculating Triangles: %u / %u\r",
                threadInfo.cntProcess, m->numTriangles());
            threadInfo.mtxProgress.unlock();

            i = 0;
        }
    }
}

int PRT::preComputeGeomCoeff(Scene *s, bool useBackup, bool updateVertices, bool parallel)
{
    bool cfUpdate = true; //backup file need update?

    if (!s)
        return -1;

    if (!sampler)
        return -1;

    /* make sure that the BVH is built */
    if (!s->bvh)
        s->buildBVH();

    scene = s;

    if (useBackup)
        cfUpdate = !loadCoeffFromFile();

    for (uint32_t i = 0; i < s->numMeshes() && cfUpdate; i++)
    {
        Mesh *m = &s->mesh[i];

        // computation done in serial
        if(!parallel)
        {
            for (uint32_t j = 0; j < s->mesh[i].numTriangles(); j++)
            {
                uint32_t &iv1 = m->triangle[j].v1;
                uint32_t &iv2 = m->triangle[j].v2;
                uint32_t &iv3 = m->triangle[j].v3;

                if (m->vertex[iv1].sh_coeff.size() == 0)
                {
                    preComputeVertexCoeff(m->vertex[iv1], i, j);
                }

                if (m->vertex[iv2].sh_coeff.size() == 0)
                {
                    preComputeVertexCoeff(m->vertex[iv2], i, j);
                }

                if (m->vertex[iv3].sh_coeff.size() == 0)
                {
                    preComputeVertexCoeff(m->vertex[iv3], i, j);
                }

                if(j%100==0)
                printf("Transfer Coefficients - Calculating: %u_%u out %u_%u\r",
                    i, j, (uint32_t)s->numMeshes(), (uint32_t)m->numTriangles());
            }
        }
        // Parallel approach
        else
        {
            threadInfo.vtxVisited.clear();
            threadInfo.vtxVisited.resize(m->numVertices());
            threadInfo.cntProcess = 0;

            uint32_t numProc = std::thread::hardware_concurrency();
            uint32_t nThread;

            if (numProc <= 0)
                nThread = 1;
            else
                nThread = numProc;

            uint32_t split = (uint32_t)ceilf((float)m->numTriangles() / (float)nThread);

            // if the number of triangles is too low
            // just put all of them into one single thread
            if (split <= 1)
            {
                nThread = 1;
                split = m->numTriangles();
            }

            std::cout << "\tComputing Mesh " << i << "/" << s->numMeshes() << std::endl;

            std::vector<std::thread> threads;

            // dispatch working threads
            uint32_t k = 0;
            for (uint32_t j = 0, k = 0; j < nThread; k += split, j++) {
                threads.push_back(std::thread(&PRT::threadWorker, this, m, k, k + split));
            }

            // wait until all threads are done with their work
            for (uint32_t j = 0; j < nThread; j++)
                threads[j].join();
            
            std::cout << std::endl;
        }
    }
    if (cfUpdate)
        saveCoeffToFile();

    if (updateVertices == true)
        computeLightIntensityAtVertices();

    return 0;
}
