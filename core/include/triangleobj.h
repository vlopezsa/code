#pragma once

#include "Falcor.h"

#include "Object.h"

class TriangleObj :
    public Object
{
private:
    vec4 centroid;
    BBox    bbox;

    void calculateCentroid();
    void calculateBBox();

public:
    TriangleObj(const void *m, uint32_t meshId, uint32_t triId);
    ~TriangleObj();

    uint32_t meshId;
    uint32_t triId;

    //! All "Objects" must be able to test for intersections with rays.
    bool getIntersection(
        const Ray& ray,
        IntersectionInfo* intersection) const;

    //! Return an object normal based on an intersection
    vec4 getNormal(const IntersectionInfo& I) const;

    //! Return a bounding box for this object
    BBox getBBox() const;

    //! Return the centroid for this object. (Used in BVH Sorting)
    vec4 getCentroid() const;
};
