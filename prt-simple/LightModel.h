#pragma once

#include "util.h"
#include "ModelLoader.h"
#include "SHSampler.h"

typedef struct {
    unsigned char *data;
    int  w, h;
}tLightProbe;

typedef struct{
    int     nCoeff;
    tPixel3 **coeff;
} tLightCoeff;

class CLightModel
{
private:
    int nBands;

    tLightProbe lightProbe;

    tModel      *geometry;

    tLightCoeff *lightCoeff;
    tLightCoeff *transCoeff;

    CSHSampler  shSampler;

private:
    void castLightFromProbe(tPixel3 *color, Point3D *dir);

    void projectLight(tPixel3 **coeff);
    void projectShadow(tPixel3 **coeff, int mIdx, int vIdx);

    void computeLightCoefficients();

    void computeTransferCoefficients();

    bool Visibility(int vIdx, int mIdx, Point3D *dir);

public:
    CLightModel();
    ~CLightModel();

    void setNumberBands(int bands);

    void setGeometry(tModel *model);

    int setLightProbe(unsigned char *data, int width, int height);

    int setLightProbeFromFile(char *file);

    int  computeCoefficients();

    void evaluatePRT(tPixel3 *p, int mIdx, int v0, int v1, int v2);
};
