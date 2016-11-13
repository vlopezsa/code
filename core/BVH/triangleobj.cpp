#include "triangleobj.h"

TriangleObj::TriangleObj(const Mesh *m, uint32_t meshId, uint32_t triId)
    : Object(),
      mesh(m),
      meshId(meshId),
      triId(triId)
{
    calculateBBox();
    calculateCentroid();
}

TriangleObj::~TriangleObj()
{
}

void TriangleObj::calculateCentroid()
{
    uint32_t iv1, iv2, iv3;

    iv1 = mesh->triangle[triId].v1;
    iv2 = mesh->triangle[triId].v2;
    iv3 = mesh->triangle[triId].v3;

    centroid = (
        mesh->vertex[iv1].position +
        mesh->vertex[iv2].position +
        mesh->vertex[iv3].position)
        / 3.0f;
}

void TriangleObj::calculateBBox()
{
    uint32_t iv1, iv2, iv3;

    iv1 = mesh->triangle[triId].v1;
    iv2 = mesh->triangle[triId].v2;
    iv3 = mesh->triangle[triId].v3;

    const Vector4 &v0 = mesh->vertex[iv1].position;
    const Vector4 &v1 = mesh->vertex[iv2].position;
    const Vector4 &v2 = mesh->vertex[iv3].position;

    Vector4 mn, mx;

    mn = vmin(v0, v1);
    mn = vmin(mn, v2);

    mx = vmax(v0, v1);
    mx = vmax(mx, v2);

    bbox.min = mn;
    bbox.max = mx;
    bbox.extent = (mx - mn);
}

bool TriangleObj::getIntersection(const Ray & ray, IntersectionInfo * intersection) const
{
    Vector4 e1, e2, p1, i1, t1, q1;
    float det = 0.0f;

    uint32_t iv1, iv2, iv3;

    iv1 = mesh->triangle[triId].v1;
    iv2 = mesh->triangle[triId].v2;
    iv3 = mesh->triangle[triId].v3;

    const Vector4 &v0 = mesh->vertex[iv1].position;
    const Vector4 &v1 = mesh->vertex[iv2].position;
    const Vector4 &v2 = mesh->vertex[iv3].position;

    const Vector4 &p = ray.o;
    const Vector4 &d = ray.d;

    e1 = v1 - v0;
    e2 = v2 - v0;

    p1 = d^e2;

    float a = e1 * p1;

    if (a > -0.00001 && a < 0.00001)
        return false;

    det = 1.0f / a;

    t1 = p - v0;

    i1.x = t1 * p1;
    i1.x *= det;

    if (i1.x < 0.0f || i1.x > 1.0f)
        return false;

    q1 = t1 ^ e1;

    i1.y = d * q1;
    i1.y *= det;

    if (i1.y < 0.0f || (i1.x + i1.y) > 1.0f)
        return false;

    i1.z = e2 * q1;
    i1.z *= det;

    if (i1.z > 0.000001f)
    {
        intersection->object = this;

        intersection->t = i1.z;

        intersection->bary.x = i1.x;
        intersection->bary.y = i1.y;
        intersection->bary.z = 1.0f - i1.x - i1.y;
        return true;
    }

    return false;
}

Vector4 TriangleObj::getNormal(const IntersectionInfo& I) const
{
    uint32_t iv1, iv2, iv3;

    iv1 = mesh->triangle[triId].v1;
    iv2 = mesh->triangle[triId].v2;
    iv3 = mesh->triangle[triId].v3;

    const Vector4 &v0 = mesh->vertex[iv1].normal;
    const Vector4 &v1 = mesh->vertex[iv2].normal;
    const Vector4 &v2 = mesh->vertex[iv3].normal;


    Vector4 vSum = (v0*I.bary.z) + (v1*I.bary.x) + (v2*I.bary.y);

    return normalize(vSum);
}

BBox TriangleObj::getBBox() const
{
    return this->bbox;
}

Vector4 TriangleObj::getCentroid() const
{
    return this->centroid;
}
