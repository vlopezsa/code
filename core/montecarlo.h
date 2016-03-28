#pragma once
#include "sampler.h"
class MonteCarlo :
    public Sampler
{
protected:
    virtual void calculateSamples();

public:
    MonteCarlo(unsigned int numSamples);
    ~MonteCarlo();
};

