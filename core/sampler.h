#pragma once

#include <vector>

#include "graphics.h"

typedef struct
{
    Vector3 Cartesian;
    Vector2 Spherical;
    struct {
        unsigned int x;
        unsigned int y;
    } Square;
}SamplePoint;

class Sampler
{
protected:
    unsigned int _numSamples;
    std::vector<SamplePoint> _Samples;

    virtual void calculateSamples() = 0;

public:
    const unsigned int &numSamples;
    const std::vector<SamplePoint> &Samples;

public:
    Sampler(unsigned int numSamples);

    ~Sampler();
};
