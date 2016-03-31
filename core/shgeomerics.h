#pragma once


#include "sphericalharmonic.h"

class SHGeomerics :
    public SphericalHarmonic
{
private:
    float calculateBasis(unsigned int l, int m, const SamplePoint *s);
    void  calculateNumberBaseCoeff();

public:
    SHGeomerics(Sampler *s);
    SHGeomerics(Sampler *sampler, unsigned int numBands);
    ~SHGeomerics();

    void calculateScaleFactors();
};