#pragma once

#include "Object.h"
#include "mesh.h"

class TriangleObj :
    public Object
{
private:
    const Mesh *mesh;

    Vector4 centroid;
    BBox    bbox;

    void calculateCentroid();
    void calculateBBox();

public:
    TriangleObj(const Mesh *m, uint32_t meshId, uint32_t triId);
    ~TriangleObj();

    uint32_t meshId;
    uint32_t triId;

    //! All "Objects" must be able to test for intersections with rays.
    bool getIntersection(
        const Ray& ray,
        IntersectionInfo* intersection) const;

    //! Return an object normal based on an intersection
    Vector4 getNormal(const IntersectionInfo& I) const;

    //! Return a bounding box for this object
    BBox getBBox() const;

    //! Return the centroid for this object. (Used in BVH Sorting)
    Vector4 getCentroid() const;
};
