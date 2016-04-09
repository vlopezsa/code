#include "sampler.h"

Sampler::Sampler(uint32_t numSamples) :
    numSamples(_numSamples),
    Samples(_Samples)
{
    this->_numSamples = numSamples;
    this->_Samples.resize(numSamples);
}

Sampler::~Sampler()
{
}