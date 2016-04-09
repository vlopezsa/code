#ifndef BBox_h
#define BBox_h

#include "Ray.h"
#include "Vector4.h"
#include <stdint.h>

struct BBox {
  Vector4 min, max, extent;
  BBox() { }
  BBox(const Vector4& min, const Vector4& max);
  BBox(const Vector4& p);

  bool intersect(const Ray& ray, float *tnear, float *tfar) const;
  void expandToInclude(const Vector4& p);
  void expandToInclude(const BBox& b);
  uint32_t maxDimension() const;
  float surfaceArea() const;
};

#endif
