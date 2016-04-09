#pragma once

#include <vector>

#include "graphics.h"

typedef struct
{
    Vector3 Cartesian;
    Vector2 Spherical;
    struct {
        uint32_t x;
        uint32_t y;
    } Square;
}SamplePoint;

class Sampler
{
protected:
    uint32_t _numSamples;
    std::vector<SamplePoint> _Samples;

    virtual void calculateSamples() = 0;

public:
    const uint32_t &numSamples;
    const std::vector<SamplePoint> &Samples;

public:
    Sampler(uint32_t numSamples);

    ~Sampler();
};
