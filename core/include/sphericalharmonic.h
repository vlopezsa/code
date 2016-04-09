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
    uint32_t       _numBands;
    SHImplementation   _shType;
  
    virtual float calculateBasis(uint32_t l, int m, const SamplePoint *s);
    virtual void  calculateNumberBaseCoeff();

protected:
    uint32_t        _numBaseCoeff;
    std::vector<float>  _scaleFactor;
    Sampler             *_Sampler;

public:
    const std::vector<std::vector<float>> &Coefficient;
    const uint32_t  &numBands;  // Number of bands
    const uint32_t  &numBaseCoeff; // Number of basis coeff
    const std::vector<float> &scaleFactor;

    const SHImplementation  &shType;

public:
    SphericalHarmonic(SHImplementation type, Sampler *sampler);
    SphericalHarmonic(SHImplementation type, Sampler *sampler, uint32_t numBands);
    ~SphericalHarmonic();

    void setNumBands(uint32_t bands);

    void calculateCoefficients();

    bool scaleFunctionCoeff(std::vector<float> &fCoeff);
    bool scaleFunctionCoeff(std::vector<Vector3> &fCoeff);

    virtual void calculateScaleFactors() { }
};
