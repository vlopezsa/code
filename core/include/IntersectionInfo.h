#ifndef IntersectionInfo_h_
#define IntersectionInfo_h_

#include "vector4.h"
#include "vector3.h"

class Object;

struct IntersectionInfo {
  float t; // Intersection distance along the ray
  const Object* object; // Object that was hit
  Vector4 hit; // Location of the intersection

  Vector3 bary;
};

#endif
