#pragma once

#include "util.h"
#include "ModelLoader.h"
#include "SHSampler.h"

typedef struct {
    unsigned char *data;
    int  w, h;
}tLightProbe;

class CLightModel
{
private:
    int nBands;
    int nBands2;
    int nSamples;

    tLightProbe lightProbe;

    tModel      *geometry;

    tPixel3     *lightCoeff; // Array of SH function
    tPixel3   ***transCoeff; // Array of meshes, Array of Vertices, Array of SH function

    CSHSampler  shSampler;

private:
    void allocateMemory();

    void deallocateMemory();

    void castLightFromProbe(tPixel3 *color, Point3D *dir);

    void projectLight(tPixel3 *coeff);
    void projectShadow(tPixel3 *coeff, int mIdx, int vIdx);

    void computeLightCoefficients();

    void computeTransferCoefficients();

    bool loadTranferCoefficients();

    bool saveTransferCoefficients();

    bool Visibility(int vIdx, int mIdx, Point3D *dir);

public:
    CLightModel();
    ~CLightModel();

    void setGeometry(tModel *model);

    int setLightProbe(unsigned char *data, int width, int height);

    int setLightProbeFromFile(char *file);

    int  computeCoefficients(int samples=64, int bands=2, bool force=false);

    void evaluatePRT(tPixel3 *p, int mIdx, int vIdx);

    void finish();
};
