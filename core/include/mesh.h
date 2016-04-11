#pragma once

#include <vector>

#include <assimp\scene.h>

#include "graphics.h"

typedef struct
{
    union
    {
        uint32_t i[3];
        struct { uint32_t a, b, c; };
        struct { uint32_t v1, v2, v3; };
    };

    Vector3 normal;
}Triangle;

class Vertex {
public:
    Vertex() {}
    Vertex(const Vertex &v)
    {
        position = v.position;
        normal = v.normal;
        diffuse = v.diffuse;

        sh_coeff.clear();
        sh_coeff = v.sh_coeff;
    }

    ~Vertex() {}

    Vector3 position;
    Vector3 normal;
    Vector2 tex;
    Color3  diffuse;
    std::vector<float> sh_coeff;
};

class Mesh 
{
public:
    std::vector<Vertex>       vertex;
    std::vector<uint32_t> index;
    std::vector<Triangle>     triangle;

    uint32_t        materialIdx;

public:
    Mesh();
    Mesh(const Mesh &m)
    {
        vertex.clear();
        index.clear();
        triangle.clear();

        vertex = m.vertex;
        index = m.index;
        triangle = m.triangle;
    }
    ~Mesh();

    void setNumVertices(int numVertex);
    void setNumTriangles(int numTriangle);

    void addVertex(int pos, Vertex &v);
    void addTriangle(int pos, Triangle &t);

    void importAIMesh(const aiMesh *dstMesh);

    uint32_t numVertices() { return (uint32_t)vertex.size(); }
    uint32_t numTriangles() { return (uint32_t)triangle.size(); }
    uint32_t numIndices() { return (uint32_t)index.size(); }
};