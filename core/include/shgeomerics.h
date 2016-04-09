#pragma once


#include "sphericalharmonic.h"

class SHGeomerics :
    public SphericalHarmonic
{
private:
    float calculateBasis(uint32_t l, int m, const SamplePoint *s);
    void  calculateNumberBaseCoeff();

public:
    SHGeomerics(Sampler *s);
    SHGeomerics(Sampler *sampler, uint32_t numBands);
    ~SHGeomerics();

    void calculateScaleFactors();
};