#pragma once


#include "sphericalharmonic.h"

class SHStandard :
    public SphericalHarmonic
{
private:
    float calculateBasis(unsigned int l, int m, const SamplePoint *s);
    void  calculateNumberBaseCoeff();

public:
    SHStandard(Sampler *s);
    SHStandard(Sampler *sampler, unsigned int numBands);
    ~SHStandard();

    void calculateScaleFactors();
};