#include "VectorMath.h"
#include <math.h>
#include <stdio.h>

#define EPSILON 0.000001
//#define EPSILON 0.001

void Copy(GLfloat *out_Destination, GLfloat *in_Origin)
{
	out_Destination[0] = in_Origin[0];
	out_Destination[1] = in_Origin[1];
	out_Destination[2] = in_Origin[2];
}

// Computes out_V3 = in_V1 cross in_V2
void CrossProduct(GLfloat *out_V3, GLfloat *in_V1, GLfloat *in_V2)
{
	// v1[0] v1[1] v1[2]
	// v2[0] v2[1] v2[2]
    out_V3[0] = in_V1[1]*in_V2[2] - in_V1[2]*in_V2[1];
	out_V3[1] = in_V1[2]*in_V2[0] - in_V1[0]*in_V2[2];
	out_V3[2] = in_V1[0]*in_V2[1] - in_V1[1]*in_V2[0];
}

// Computes the dot product of the given input vectors
GLfloat DotProduct(GLfloat *in_V1, GLfloat *in_V2)
{
	return	in_V1[0] * in_V2[0] +
			in_V1[1] * in_V2[1] +
			in_V1[2] * in_V2[2];
}

GLfloat Length(GLfloat *in_V)
{
	GLfloat x = in_V[0];
	GLfloat y = in_V[1];
	GLfloat z = in_V[2];
	return sqrt(x*x + y*y + z*z);
}

void Normalize(GLfloat *inout_V)
{
	GLfloat V = Length(inout_V);
	if (V > 0.00001f)
	{
		inout_V[0] /= V;
		inout_V[1] /= V;
		inout_V[2] /= V;
	}
}

// Returns out_V3 = in_V1 - in_V2
void Substract(GLfloat *out_V3, GLfloat *in_V1, GLfloat *in_V2)
{
	out_V3[0] = in_V1[0] - in_V2[0];
	out_V3[1] = in_V1[1] - in_V2[1];
	out_V3[2] = in_V1[2] - in_V2[2];
}

void Add(GLfloat *out_V3, GLfloat *in_V1, GLfloat *in_V2)
{
	out_V3[0] = in_V1[0] + in_V2[0];
	out_V3[1] = in_V1[1] + in_V2[1];
	out_V3[2] = in_V1[2] + in_V2[2];
}

void Invert(GLfloat *inout_Vector)
{
	inout_Vector[0] = -inout_Vector[0];
	inout_Vector[1] = -inout_Vector[1];
	inout_Vector[2] = -inout_Vector[2];
}

void Multiply(GLfloat in_Scalar, GLfloat *inout_Vector)
{
	inout_Vector[0] *= in_Scalar;
	inout_Vector[1] *= in_Scalar;
	inout_Vector[2] *= in_Scalar;
}

void Reflect(GLfloat *out_Reflected, GLfloat *in_Normal, GLfloat *in_Vector)
{
	GLfloat Normal[3];
	Copy(Normal, in_Normal);

	GLfloat Dot = DotProduct(Normal, in_Vector);
	Multiply(2.0f * Dot, Normal);
	Substract(out_Reflected, Normal, in_Vector);
	Normalize(out_Reflected);
}

/*
	Computes the intersection between a ray with origin at "in_Origin" and unitary direction "in_Direction" against a triangle
	with vertices "in_Vertex0", "in_Vertex1" and "in_Vertex2". If an intersection is found, "true" is returned along with
	the barycentric coordinates of the intersection (out_u, out_v, out_t); otherwise, "false" is returned.
	
	The barycentric coordinates are such that the intersection point is:
	(1 - u - v)*v0 + u*v1 + v*v2
*/
bool RayTriIntersect(GLfloat *in_Origin, GLfloat *in_Direction, 
					 GLfloat *in_Vertex0, GLfloat *in_Vertex1, GLfloat *in_Vertex2,
					 GLfloat *out_u, GLfloat *out_v, GLfloat *out_t)
{
	GLfloat		e1[3];
	GLfloat		e2[3];
	GLfloat		p[3];
	GLfloat		q[3];
	GLfloat		s[3];
	GLfloat		a;
	GLfloat		f;

	// Initialize the result
	*out_u = *out_v = *out_t = 0.0f;

	// e1 = v1 - v0
	Substract(e1, in_Vertex1, in_Vertex0);
	// e2 = v2 - v0
	Substract(e2, in_Vertex2, in_Vertex0);
	// p = d x e2
	CrossProduct(p, in_Direction, e2);
	// a = e1 . p
	a = DotProduct(e1, p);
	if (a > -EPSILON && a < EPSILON)
	{
		//printf("EXIT POINT 0\n");
		return false;
	}

	f = 1.0f / a;
	// s = o - v0
	Substract(s, in_Origin, in_Vertex0);

	// u = f (s . p)
	*out_u = f * DotProduct(s, p);
	if (*out_u < 0.0f - EPSILON || *out_u > 1.0f + EPSILON)
	{
		//printf("EXIT POINT 1\n");
		return false;
	}

	// q = s x e1
	CrossProduct(q, s, e1);

	*out_v = f * DotProduct(in_Direction, q);
	if (*out_v < 0.0f - EPSILON || *out_u + *out_v > 1.0f + EPSILON)
	{
		//printf("EXIT POINT 2. *out_v: %f, *out_u + *out_v: %f\n", *out_v, *out_u + *out_v);
		return false;
	}

	*out_t = f * DotProduct(e2, q);
	return true;
}

// (1 - u - v)*v0 + u*v1 + v*v2
void Interpolate(GLfloat *out_V, GLfloat *in_V0, GLfloat *in_V1, GLfloat *in_V2,
				 float in_u, float in_v)
{
	float w = 1.0f - in_u - in_v;
	for (register int i = 0; i < 3; i++)
		out_V[i] = w*in_V0[i] + in_u*in_V1[i] + in_v*in_V2[i];
}

bool RayTriIntersectAlways(GLfloat *in_Origin, GLfloat *in_Direction, 
					 GLfloat *in_Vertex0, GLfloat *in_Vertex1, GLfloat *in_Vertex2,
					 GLfloat *out_u, GLfloat *out_v, GLfloat *out_t)
{
	GLfloat		e1[3];
	GLfloat		e2[3];
	GLfloat		p[3];
	GLfloat		q[3];
	GLfloat		s[3];
	GLfloat		a;
	GLfloat		f;

	// Initialize the result
	*out_u = *out_v = *out_t = 0.0f;

	// e1 = v1 - v0
	Substract(e1, in_Vertex1, in_Vertex0);
	// e2 = v2 - v0
	Substract(e2, in_Vertex2, in_Vertex0);
	// p = d x e2
	CrossProduct(p, in_Direction, e2);
	// a = e1 . p
	a = DotProduct(e1, p);
	if (a > -EPSILON && a < EPSILON)
	//if (a == 0.0f)
	{
		return false;
	}

	f = 1.0f / a;
	// s = o - v0
	Substract(s, in_Origin, in_Vertex0);

	// u = f (s . p)
	*out_u = f * DotProduct(s, p);

	// q = s x e1
	CrossProduct(q, s, e1);

	*out_v = f * DotProduct(in_Direction, q);

	*out_t = f * DotProduct(e2, q);
	return true;
}

void rotateVector(float *x, float *y, float *z,
				  float ax, float ay, float az, float halfsin, float halfcos) 
{
  float wx = halfcos*(*x) - halfsin*((*z)*ay - (*y)*az);
  float wy = halfcos*(*y) - halfsin*((*x)*az - (*z)*ax);
  float wz = halfcos*(*z) - halfsin*((*y)*ax - (*x)*ay);
  float w = halfsin*((*x)*ax + (*y)*ay + (*z)*az);

  (*x) = halfcos * wx + halfsin * (ax*w - ay*wz + az*wy);
  (*y) = halfcos * wy + halfsin * (ay*w - az*wx + ax*wz);
  (*z) = halfcos * wz + halfsin * (az*w - ax*wy + ay*wx);
}

void TransformPoint(GLfloat *inout_Point, GLfloat *in_Matrix)
{
	GLfloat Temp[4];
	register int Ofs = 0;
	for (register int i = 0; i < 4; i++)
	{
		Temp[i] = 0.0f;
		Ofs = i;
		for (register int j = 0; j < 4; j++)
		{
			if (j < 3)
			{
				Temp[i] += inout_Point[j] * in_Matrix[Ofs];
			}
			else
			{
				Temp[i] += in_Matrix[Ofs];
			}
			Ofs += 4;
		}
	}
	Copy(inout_Point, Temp);
}

GLfloat **CreateMatrix()
{
	GLfloat **Result = new float *[3];
	for (int i = 0; i < 3; i++)
	{
		Result[i] = new float[3];
	}
	return Result;
}

void DeleteMatrix(GLfloat **in_Matrix)
{
	for (int i = 0; i < 3; i++)
	{
		delete[] in_Matrix[i];
	}
	delete[] in_Matrix;
}

void MultiplyMatrix(GLfloat *out_Point, GLfloat **in_Matrix, GLfloat *in_Point)
{
	for (int i = 0; i < 3; i++)
	{
		float Sum = 0.0f;
		for (int j = 0; j < 3; j++)
		{
			Sum += in_Matrix[i][j]*in_Point[j];
		}
		out_Point[i] = Sum;
	}
}

void SetIdentityMatrix(GLfloat **out_Matrix)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			out_Matrix[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
}

// c -s  0
// s  c  0
// 0  0  1
void SetRotationZ(GLfloat **out_Matrix, GLfloat in_CosAng, GLfloat in_SinAng)
{
   SetIdentityMatrix(out_Matrix);
   out_Matrix[0][0] = in_CosAng;
   out_Matrix[0][1] = -in_SinAng;

   out_Matrix[1][0] = in_SinAng;
   out_Matrix[1][1] = in_CosAng;
}

// 1  0  0
// 0  c -s
// 0  s  c 
void SetRotationX(GLfloat **out_Matrix, GLfloat in_CosAng, GLfloat in_SinAng)
{
   SetIdentityMatrix(out_Matrix);
   out_Matrix[1][1] = in_CosAng;
   out_Matrix[1][2] = -in_SinAng;

   out_Matrix[2][1] = in_SinAng;
   out_Matrix[2][2] = in_CosAng;
}

// c  0 -s
// 0  1  0
// s  0  c 
void SetRotationY(GLfloat **out_Matrix, GLfloat in_CosAng, GLfloat in_SinAng)
{
   SetIdentityMatrix(out_Matrix);
   out_Matrix[0][0] = in_CosAng;
   out_Matrix[0][2] = -in_SinAng;

   out_Matrix[2][0] = in_SinAng;
   out_Matrix[2][2] = in_CosAng;
}

void Transpose(GLfloat **inout_Matrix)
{
	GLfloat Temp;
	for (int i = 0; i < 3; i++)
	{
		for (int j = i+1; j < 3; j++)
		{
			Temp = inout_Matrix[i][j];
			inout_Matrix[i][j] = inout_Matrix[j][i];
			inout_Matrix[j][i] = Temp;
		}
	}
}

void MultiplyMatrix(GLfloat **out_Matrix, GLfloat **in_Matrix1, GLfloat **in_Matrix2)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			float Sum = 0.0f;
			for (int k = 0; k < 3; k++)
			{
				Sum += in_Matrix1[i][k]*in_Matrix2[k][j];
			}
			out_Matrix[i][j] = Sum;
		}
	}
}

// Assumes in_Vector is normalized
GLfloat **ComputeAlignZMatrix(GLfloat *in_Vector)
{
	GLfloat x = in_Vector[0];
	GLfloat y = in_Vector[1];
	GLfloat z = in_Vector[2];

	GLfloat **Result = CreateMatrix();
	GLfloat **RotX = CreateMatrix();
	GLfloat **RotY = CreateMatrix();

	// Rotate about X
	GLfloat H = sqrt(y*y + z*z);
	if (H > 0.0001f)
	{
		SetRotationX(RotX, z/H, y/H);
	}
	else
	{
		SetIdentityMatrix(RotX);
	}

	SetRotationY(RotY, H, x);
	MultiplyMatrix(Result, RotY, RotX);
	DeleteMatrix(RotX);
	DeleteMatrix(RotY);
	return Result;
}

void GetClosestPointToTheOrigin(GLfloat *out_Point, GLfloat *in_PointOnLine, GLfloat *in_Direction)
{
	GLfloat u = - in_PointOnLine[0] * in_Direction[0] - in_PointOnLine[1] * in_Direction[1] - in_PointOnLine[2] * in_Direction[2];
	out_Point[0] = in_PointOnLine[0] + u*in_Direction[0];
	out_Point[1] = in_PointOnLine[1] + u*in_Direction[1];
	out_Point[2] = in_PointOnLine[2] + u*in_Direction[2];
}