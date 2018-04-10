#ifndef Ray_h
#define Ray_h

#include "Falcor.h"

struct Ray {
  vec4 o; // Ray Origin
  vec4 d; // Ray Direction
  vec4 inv_d; // Inverse of each Ray Direction component

  Ray(const vec4& o, const vec4& d)
    : o(o), d(d), inv_d(vec4(1,1,1,1)/d) { }
};

#endif
