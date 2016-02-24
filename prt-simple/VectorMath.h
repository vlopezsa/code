#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <windows.h>
#include <GL\gl.h>

const float PI = 3.141592f;
const float HALF_PI = PI / 2.0f;
const float TWO_PIS = 2*PI;

void Copy(GLfloat *out_Destination, GLfloat *in_Origin);
void CrossProduct(GLfloat *out_V3, GLfloat *in_V1, GLfloat *in_V2);
GLfloat DotProduct(GLfloat *in_V1, GLfloat *in_V2);
GLfloat Length(GLfloat *inout_V);
void Normalize(GLfloat *inout_V);
void Substract(GLfloat *out_V3, GLfloat *in_V1, GLfloat *in_V2);
void Add(GLfloat *out_V3, GLfloat *in_V1, GLfloat *in_V2);
void Multiply(GLfloat in_Scalar, GLfloat *inout_Vector);
void Invert(GLfloat *inout_Vector);
void Reflect(GLfloat *out_Reflected, GLfloat *in_Normal, GLfloat *in_Vector);

bool RayTriIntersect(GLfloat *in_Origin, GLfloat *in_Direction, 
					 GLfloat *in_Vertex0, GLfloat *in_Vertex1, GLfloat *in_Vertex2,
					 GLfloat *out_u, GLfloat *out_v, GLfloat *out_t);

bool RayTriIntersectAlways(GLfloat *in_Origin, GLfloat *in_Direction, 
					 GLfloat *in_Vertex0, GLfloat *in_Vertex1, GLfloat *in_Vertex2,
					 GLfloat *out_u, GLfloat *out_v, GLfloat *out_t);

void Interpolate(GLfloat *out_V, GLfloat *in_V0, GLfloat *in_V1, GLfloat *in_V2,
				 float in_u, float in_v);

void rotateVector(float *x, float *y, float *z,
				  float ax, float ay, float az, float halfsin, float halfcos);

void TransformPoint(GLfloat *inout_Point, GLfloat *in_Matrix);

GLfloat **CreateMatrix();
void DeleteMatrix(GLfloat **in_Matrix);
void MultiplyMatrix(GLfloat *out_Point, GLfloat **in_Matrix, GLfloat *in_Point);
void MultiplyMatrix(GLfloat **out_Matrix, GLfloat **in_Matrix1, GLfloat **in_Matrix2);
void SetIdentityMatrix(GLfloat **out_Matrix);
void SetRotationX(GLfloat **out_Matrix, GLfloat in_CosAng, GLfloat in_SinAng);
void SetRotationY(GLfloat **out_Matrix, GLfloat in_CosAng, GLfloat in_SinAng);
void SetRotationZ(GLfloat **out_Matrix, GLfloat in_CosAng, GLfloat in_SinAng);
void Transpose(GLfloat **inout_Matrix);

// Assumes in_Vector is normalized
GLfloat **ComputeAlignZMatrix(GLfloat *in_Vector);

void GetClosestPointToTheOrigin(GLfloat *out_Point, GLfloat *in_PointOnLine, GLfloat *in_Direction);
#endif