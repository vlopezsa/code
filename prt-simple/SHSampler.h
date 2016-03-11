#pragma once

#include "util.h"

#include "LightModel.h"
#include "glVector.h"

typedef struct
{
    glVector  cart;
    glVector  sphr;
    float   *sh;
}tSphereSample;

class CSHSampler
{
private:
    int     nBands;
    int     nSamples;

    tSphereSample  *sphereSample;

private:
    void deleteShCoeff();

    void calculateSH(float **sh, glVector *uSphere);

public:
    CSHSampler();
    ~CSHSampler();

    void setNumberSamples(int samples, int bands);

    void calculateSamples();

    int getNumberSamples() { return nSamples; };
    int getNumberBands() { return nBands; };
    tSphereSample* getSample(int i) { return &sphereSample[i]; }

    //float getSHCoefficient();
};
