#ifndef __CRENDER_MAT44_H__
#define __CRENDER_MAT44_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrVec3;

struct CrVec4;

typedef struct CrMat44
{
	union
	{
		struct { float
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33;
		};

		float v[16];
	};

} CrMat44;

CrMat44 crMat44(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33);

void crMat44Mult(CrMat44* _out, const CrMat44* a, const CrMat44* b);

CrBool crMat44Inverse(CrMat44* _out, const CrMat44* m);

void crMat44Transpose(CrMat44* _out, const CrMat44* m);

void crMat44Transform(struct CrVec4* _out, const CrMat44* m);

//! assume the plane equation is stored as A, B, C, D
void crMat44TransformPlane(struct CrVec4* _out, const CrMat44* m);

void crMat44TransformAffineDir(struct CrVec3* _out, const CrMat44* m);

void crMat44TransformAffinePt(struct CrVec3* _out, const CrMat44* m);

void crMat44TransformAffineDirs(struct CrVec3* _out, const struct CrVec3* _in, size_t cnt, const CrMat44* m);

void crMat44TransformAffinePts(struct CrVec3* _out, const struct CrVec3* _in, size_t cnt, const CrMat44* m);

void crMat44SetIdentity(CrMat44* _out);

void crMat44SetTranslation(CrMat44* _out, const struct CrVec3* v);

void crMat44GetTranslation(struct CrVec3* v, const CrMat44* m);

void crMat44MakeTranslation(CrMat44* _out, const struct CrVec3* v);

void crMat44MakeScale(CrMat44* _out, const struct CrVec3* v);

void crMat44MakeRotationX(CrMat44* _out, float angleInDeg);

void crMat44MakeRotationY(CrMat44* _out, float angleInDeg);

void crMat44MakeRotationZ(CrMat44* _out, float angleInDeg);

void crMat44MakeRotation(CrMat44* _out, const struct CrVec3* axis, float angleInDeg);

void crMat44GetBasis(struct CrVec3* xaxis, struct CrVec3* yaxis, struct CrVec3* zaxis, const CrMat44* m);

void crMat44CameraLookAt(CrMat44* _out, const struct CrVec3* eyeAt, const struct CrVec3* lookAt, const struct CrVec3* eyeUp);

void crMat44Prespective(CrMat44* _out, float fovyDeg, float aspect, float znear, float zfar);

void crMat44PlanarReflect(CrMat44* _out, const struct CrVec3* normal, const struct CrVec3* point);

//! adjust a projection matrix to match the API's depth range
void crMat44AdjustToAPIDepthRange(CrMat44* _out);

//! adjust a projection matrix to match the API's projective texture lookup
void crMat44AdjustToAPIProjectiveTexture(CrMat44* _out);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_MAT44_H__