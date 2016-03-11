#pragma once

#include "util.h"
#include "ModelLoader.h"
#include "SHSampler.h"
#include "glVector.h"

typedef struct {
    unsigned char *data;
    int  w, h;
}tLightProbe;

class CLightModel
{
private:
    tLightProbe lightProbe;

    tModel      *geometry;

    glVector    *lightCoeff; // Array of SH function
    glVector  ***transCoeff; // Array of meshes, Array of Vertices, Array of SH function

    CSHSampler  shSampler;

    void(*progresCbk)(char *);

    void updateProgress(char *phase, float percent);

private:
    void allocateMemory();

    void deallocateMemory();

    void castLightFromProbe(glVector *color, glVector *dir);

    void projectLight(glVector *coeff);
    void projectShadow(glVector *coeff, int mIdx, int vIdx);

    void projectShadowRange(glVector *coeff, int mIdx, int vIdx_Start, int vIdx_End);

    void computeLightCoefficients();

    void computeTransferCoefficients();
    
    bool loadTranferCoefficients();

    bool saveTransferCoefficients();

    bool Visibility(int vIdx, int mIdx, glVector *dir);

public:
    CLightModel();
    ~CLightModel();

    void setGeometry(tModel *model);

    int setLightProbe(unsigned char *data, int width, int height);

    int setLightProbeFromFile(char *file);

    int  computeCoefficients(int samples=64, int bands=2, bool force=false);

    void evaluatePRT(tPixel3 *p, int mIdx, int vIdx);

    void setProgressCbk(void(*)(char *));

    void finish();

    int nBands;
    int nBands2;
    int nSamples;
};
