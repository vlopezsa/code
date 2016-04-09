#pragma once


#include "sphericalharmonic.h"

class SHStandard :
    public SphericalHarmonic
{
private:
    float calculateBasis(uint32_t l, int m, const SamplePoint *s);
    void  calculateNumberBaseCoeff();

public:
    SHStandard(Sampler *s);
    SHStandard(Sampler *sampler, uint32_t numBands);
    ~SHStandard();

    void calculateScaleFactors();
};