#include <assimp\scene.h>

#include "mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::setNumVertices(uint32_t numVertex)
{
    try {
        this->vertex.resize(numVertex);
        this->index.resize(numVertex);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::setNumTriangles(uint32_t numTriangle)
{
    try
    {
        this->triangle.resize(numTriangle);
        this->index.resize(numTriangle * 3);
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::addVertex(uint32_t pos, Vertex & v)
{
    try
    {
        vertex[pos].diffuse  = v.diffuse;
        vertex[pos].normal   = v.normal;
        vertex[pos].position = v.position;
        vertex[pos].tex      = v.tex;

        //vertex[pos].sh_coeff = v.sh_coeff;
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::addTriangle(uint32_t pos, Triangle & t)
{
    try
    {
        triangle[pos].v1 = t.v1;
        triangle[pos].v2 = t.v2;
        triangle[pos].v3 = t.v3;

        uint32_t idxPos = pos * 3;
        index[idxPos++] = t.v1;
        index[idxPos++] = t.v2;
        index[idxPos++] = t.v3;
    }
    catch (std::exception &e)
    {
        throw e;
    }
}

void Mesh::importAIMesh(const aiMesh *mesh)
{
    try 
    {
        vertex.clear();
        index.clear();
        triangle.clear();

        vertex.resize(mesh->mNumVertices);
        triangle.resize(mesh->mNumFaces);
        index.resize(mesh->mNumFaces * 3);

        uint32_t texCoordIdx = AI_MAX_NUMBER_OF_TEXTURECOORDS;
        uint32_t colorIdx = AI_MAX_NUMBER_OF_COLOR_SETS;

        for (uint32_t i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
        {
            if (mesh->HasTextureCoords(i))
            {
                texCoordIdx = i;
                break;
            }
        }

        for (uint32_t i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
        {
            if (mesh->HasVertexColors(i))
            {
                colorIdx = i;
                break;
            }
        }

        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            vertex[i].position.x = mesh->mVertices[i].x;
            vertex[i].position.y = mesh->mVertices[i].y;
            vertex[i].position.z = mesh->mVertices[i].z;

            if (mesh->HasNormals())
            {
                vertex[i].normal.x = mesh->mNormals[i].x;
                vertex[i].normal.y = mesh->mNormals[i].y;
                vertex[i].normal.z = mesh->mNormals[i].z;
            }

            if (texCoordIdx < AI_MAX_NUMBER_OF_TEXTURECOORDS)
            {
                vertex[i].tex.u = mesh->mTextureCoords[texCoordIdx][i].x;
                vertex[i].tex.v = mesh->mTextureCoords[texCoordIdx][i].y;
            }

            if (colorIdx < AI_MAX_NUMBER_OF_COLOR_SETS)
            {
                vertex[i].diffuse.r = mesh->mColors[colorIdx][i].r;
                vertex[i].diffuse.g = mesh->mColors[colorIdx][i].g;
                vertex[i].diffuse.b = mesh->mColors[colorIdx][i].b;
            }
        }

        if (mesh->HasFaces())
        {
            for (uint32_t i = 0, j=0; i < mesh->mNumFaces; i++)
            {
                triangle[i].v1 = mesh->mFaces[i].mIndices[0];
                triangle[i].v2 = mesh->mFaces[i].mIndices[1];
                triangle[i].v3 = mesh->mFaces[i].mIndices[2];

                index[j++] = mesh->mFaces[i].mIndices[0];
                index[j++] = mesh->mFaces[i].mIndices[1];
                index[j++] = mesh->mFaces[i].mIndices[2];

                triangle[i].normal = 
                   (vertex[triangle[i].v1].normal +
                    vertex[triangle[i].v1].normal +
                    vertex[triangle[i].v1].normal) / 3.0f;

                triangle[i].normal.normalize();
            }
        }        

        materialIdx = mesh->mMaterialIndex;
    }
    catch (std::exception &e)
    {
        throw e;
    }   
}
