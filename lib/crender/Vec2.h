#ifndef __CRENDER_VEC2_H__
#define __CRENDER_VEC2_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrVec2
{
	union
	{
		struct {
			float x;
			float y;
		};
		float v[2];
	};
} CrVec2;

CrVec2 crVec2(float x, float y);

const CrVec2* CrVec2_c00();
const CrVec2* CrVec2_c10();
const CrVec2* CrVec2_c01();

void crVec2_set(CrVec2* _out, float x, float y);

CrBool crVec2IsEqual(const CrVec2* a, const CrVec2* b, float epsilon);

/* return a + b */
CrVec2* crVec2Add(CrVec2* _out, const CrVec2* a, const CrVec2* b);

/* return a - b */
CrVec2* crVec2Sub(CrVec2* _out, const CrVec2* a, const CrVec2* b);

/* return a * b */
CrVec2* crVec2Mult(CrVec2* _out, const CrVec2* a, const CrVec2* b);

/* return a * b */
CrVec2* crVec2MultS(CrVec2* _out, const CrVec2* a, float b);

/* return a dot b */
float crVec2Dot(const CrVec2* a, const CrVec2* b);

/* return |a|^2 */
float crVec2SqLength(const CrVec2* a);

/* return |a| */
float crVec2Length(const CrVec2* a);

/* return |a-b| */
float crVec2Distance(const CrVec2* a, const CrVec2* b);

/* normalize a and return |a| */
float crVec2Normalize(CrVec2* a);

/* return normalized copy of a */
CrVec2 crVec2NormalizedCopy(const CrVec2* a);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_VEC2_H__
