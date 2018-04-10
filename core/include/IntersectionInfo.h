#ifndef IntersectionInfo_h_
#define IntersectionInfo_h_

#include "Falcor.h"

class Object;

struct IntersectionInfo {
  float t; // Intersection distance along the ray
  const Object* object; // Object that was hit
  vec4 hit; // Location of the intersection

  vec3 bary;
};

#endif
