#ifndef Ray_h
#define Ray_h

#include "vector4.h"

struct Ray {
  Vector4 o; // Ray Origin
  Vector4 d; // Ray Direction
  Vector4 inv_d; // Inverse of each Ray Direction component

  Ray(const Vector4& o, const Vector4& d)
    : o(o), d(d), inv_d(Vector4(1,1,1,1).cdiv(d)) { }
};

#endif
