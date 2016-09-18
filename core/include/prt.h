#pragma once

#include <vector>
#include <thread>
#include <mutex>

#include "light.h"
#include "shstandard.h"
#include "sampler.h"
#include "scene.h"


class PRT: public Light
{
private:
    SHStandard *sh;
    Sampler    *sampler;

    Scene      *scene; // external reference, dont delete in this class

    uint32_t    nBands;

    std::vector<Vector3> lightCoeff;

    struct
    {
        std::vector<bool> vtxVisited;
        uint32_t          cntProcess;

        std::mutex        mtxProgress;
        std::mutex        mtxVertex;
    }threadInfo;

private:
    void preComputeVertexCoeff(Vertex & v, uint32_t &iMesh, uint32_t &iTrian);

    void threadWorker(Mesh *m, uint32_t vIdx_Start, uint32_t vIdx_End);

    bool loadCoeffFromFile();
    bool saveCoeffToFile();

    void computeLightIntensityAtVertices();

public:
    PRT();
    PRT(Sampler *sampler);

    ~PRT();

    void setSampler(Sampler *sampler);

    Vector3 getIntensityAt(Vertex &v, bool clamp = false);

    int preComputeLight();

    /*
        useBackup - true  try to locate precomputede values for
                          the scene. If not found any, theye will
                          be recalculated and store it again.

                  - false force the recalculation of the coefficients.
                          The coefficients will be backed up into a file
                          at the end.

        updateVertices - true   calculate the intensity of light on each vertex
                                and store it in the diffuse color of the vertex
                         false  don't do a thing

    */
    int preComputeGeomCoeff(Scene *, bool useBackup = true, bool updateVertices = true, bool parallel = true);
};
