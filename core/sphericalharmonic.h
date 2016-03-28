#pragma once

#include <vector>

#include "sampler.h"

class SphericalHarmonic
{
private:
    std::vector<std::vector<float>>  _Coefficient;
    Sampler             *_Sampler;
    unsigned int        _numBands;
    unsigned int        _numBands2;
   
    void calculateCoefficients();

public:
    const std::vector<std::vector<float>> &Coefficient;
    const unsigned int  &numBands;  // Number of bands
    const unsigned int  &numBands2; // Number of bands square 

public:
    SphericalHarmonic(Sampler *sampler, unsigned int numBands);
    ~SphericalHarmonic();

};
