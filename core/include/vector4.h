#pragma once
/* From BVH implementation */

/*
Code: Fast-BVH, an optimized Bounding Volume Hierarchy
Author: Brandon Pelfrey (brandonpelfrey@gmail.com)
Date: April 17, 2012
*/

#include <string>
#include <cmath>
#include <emmintrin.h>
#include <pmmintrin.h>
#include "Log.h"

#include "vector3.h"

// SSE Vector object
class Vector4 {
public:
    // This is silly, but it still helps.
#if defined(_WIN32) || defined(__WIN32__)
    union __declspec(align(16)) {
#else
    union __attribute__((aligned(16))) {
#endif
        float  d[4];
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        __m128 m128;
    };

    Vector4() { }
    Vector4(float x, float y, float z, float w = 0.f) : m128(_mm_set_ps(w, z, y, x)) { }
    Vector4(const __m128& m128) : m128(m128) { }
    Vector4(const Vector3 &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = 1.0f;
    }
    Vector4(const Vector4 &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = v.w;
    }

    Vector4 operator+(const Vector4& b) const { return _mm_add_ps(m128, b.m128); }
    Vector4 operator-(const Vector4& b) const { return _mm_sub_ps(m128, b.m128); }
    Vector4 operator*(float b) const { return _mm_mul_ps(m128, _mm_set_ps(b, b, b, b)); }
    Vector4 operator/(float b) const { return _mm_div_ps(m128, _mm_set_ps(b, b, b, b)); }

    // Component-wise multiply and divide
    Vector4 cmul(const Vector4& b) const { return _mm_mul_ps(m128, b.m128); }
    Vector4 cdiv(const Vector4& b) const { return _mm_div_ps(m128, b.m128); }

    // dot (inner) product
    float operator*(const Vector4& b) const {
        return x*b.x + y*b.y + z*b.z;
    }

    // Cross Product
    Vector4 operator^(const Vector4& b) const {
        return _mm_sub_ps(
            _mm_mul_ps(
                _mm_shuffle_ps(m128, m128, _MM_SHUFFLE(3, 0, 2, 1)),
                _mm_shuffle_ps(b.m128, b.m128, _MM_SHUFFLE(3, 1, 0, 2))),
            _mm_mul_ps(
                _mm_shuffle_ps(m128, m128, _MM_SHUFFLE(3, 1, 0, 2)),
                _mm_shuffle_ps(b.m128, b.m128, _MM_SHUFFLE(3, 0, 2, 1)))
            );
    }

    Vector4 operator/(const Vector4& b) const { return _mm_div_ps(m128, b.m128); }

    // Handy component indexing
    float& operator[](const uint32_t i) { return (&x)[i]; }
    const float& operator[](const uint32_t i) const { return (&x)[i]; }
};

inline Vector4 operator*(float a, const Vector4&b)
{ 
    return _mm_mul_ps(_mm_set1_ps(a), b.m128);
}

// Component-wise min
inline Vector4 vmin(const Vector4& a, const Vector4& b) {
    return _mm_min_ps(a.m128, b.m128);
}

// Component-wise max
inline Vector4 vmax(const Vector4& a, const Vector4& b) {
    return _mm_max_ps(a.m128, b.m128);
}

// Length of a vector
inline float length(const Vector4& a) {
    return sqrtf(a*a);
}

// Make a vector unit length
inline Vector4 normalize(const Vector4& in) {
    Vector4 a = in;
    a.w = 0.f;

    __m128 D = a.m128;
    D = _mm_mul_ps(D, D);
    D = _mm_hadd_ps(D, D);
    D = _mm_hadd_ps(D, D);

    // 1 iteration of Newton-raphson -- Idea from Intel's Embree.
    __m128 r = _mm_rsqrt_ps(D);
    r = _mm_add_ps(
        _mm_mul_ps(_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f), r),
        _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(D, _mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f)), r), _mm_mul_ps(r, r)));

    return _mm_mul_ps(a.m128, r);
}
