#ifndef Object_h_
#define Object_h_

#include "Falcor.h"

#include "IntersectionInfo.h"
#include "Ray.h"
#include "BBox.h"


class Object {
public:
    Object() {}
    ~Object() {}

    //! All "Objects" must be able to test for intersections with rays.
    virtual bool getIntersection(
          const Ray& ray,
        IntersectionInfo* intersection)
    const = 0;

    //! Return an object normal based on an intersection
    virtual vec4 getNormal(const IntersectionInfo& I) const = 0;

    //! Return a bounding box for this object
    virtual BBox getBBox() const = 0;

    //! Return the centroid for this object. (Used in BVH Sorting)
    virtual vec4 getCentroid() const = 0;
};

#endif
