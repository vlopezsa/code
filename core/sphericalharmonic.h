#pragma once

#include <vector>

#include "sampler.h"
#include "graphics.h"

#define __SH_IMP_TYPE_DEF {SH_Unknown,SH_Standard,SH_Geomerics}

enum SHImplementation __SH_IMP_TYPE_DEF;

class SphericalHarmonic
{
private:
    std::vector<std::vector<float>>  _Coefficient;
    unsigned int       _numBands;
    SHImplementation   _shType;
  
    virtual float calculateBasis(unsigned int l, int m, const SamplePoint *s);
    virtual void  calculateNumberBaseCoeff();

protected:
    unsigned int        _numBaseCoeff;
    std::vector<float>  _scaleRec;
    Sampler             *_Sampler;

public:
    const std::vector<std::vector<float>> &Coefficient;
    const unsigned int  &numBands;  // Number of bands
    const unsigned int  &numBaseCoeff; // Number of basis coeff
    const std::vector<float> &scaleRec;

    const SHImplementation  &shType;

public:
    SphericalHarmonic(SHImplementation type, Sampler *sampler);
    SphericalHarmonic(SHImplementation type, Sampler *sampler, unsigned int numBands);
    ~SphericalHarmonic();

    void setNumBands(unsigned int bands);

    void calculateCoefficients();

    virtual float getScaleFactor() { return 1.0f; }
};
