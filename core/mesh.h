#pragma once

#include <vector>

#include <assimp\scene.h>

#include "graphics.h"

typedef struct
{
    union
    {
        int i[3];
        struct { int a, b, c; };
        struct { int v1, v2, v3; };
    };
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
    std::vector<unsigned int> index;
    std::vector<Triangle>     triangle;

    unsigned int        materialIdx;

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

    size_t numVertices() { return vertex.size(); }
    size_t numTriangles() { return triangle.size(); }
    size_t numIndices() { return index.size(); }
};