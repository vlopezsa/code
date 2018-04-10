#ifndef BBox_h
#define BBox_h

#include "Falcor.h"

#include "Ray.h"
#include <stdint.h>

struct BBox {
  vec4 min, max, extent;
  BBox() { }
  BBox(const vec4& min, const vec4& max);
  BBox(const vec4& p);

  bool intersect(const Ray& ray, float *tnear, float *tfar) const;
  void expandToInclude(const vec4& p);
  void expandToInclude(const BBox& b);
  uint32 maxDimension() const;
  float surfaceArea() const;
};

#endif
