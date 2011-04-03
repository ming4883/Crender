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

CR_API CrVec2 crVec2(float x, float y);

CR_API const CrVec2* CrVec2_c00();
CR_API const CrVec2* CrVec2_c10();
CR_API const CrVec2* CrVec2_c01();

#define crVec2Set(_out, x, y) {_out.x = x; _out.y = y;}

CR_API CrBool crVec2IsEqual(const CrVec2* a, const CrVec2* b, float epsilon);

/* return a + b */
CR_API CrVec2* crVec2Add(CrVec2* _out, const CrVec2* a, const CrVec2* b);

/* return a - b */
CR_API CrVec2* crVec2Sub(CrVec2* _out, const CrVec2* a, const CrVec2* b);

/* return a * b */
CR_API CrVec2* crVec2Mult(CrVec2* _out, const CrVec2* a, const CrVec2* b);

/* return a * b */
CR_API CrVec2* crVec2MultS(CrVec2* _out, const CrVec2* a, float b);

/* return a dot b */
CR_API float crVec2Dot(const CrVec2* a, const CrVec2* b);

/* return |a|^2 */
CR_API float crVec2SqLength(const CrVec2* a);

/* return |a| */
CR_API float crVec2Length(const CrVec2* a);

/* return |a-b| */
CR_API float crVec2Distance(const CrVec2* a, const CrVec2* b);

/* normalize a and return |a| */
CR_API float crVec2Normalize(CrVec2* a);

/* return normalized copy of a */
CR_API CrVec2 crVec2NormalizedCopy(const CrVec2* a);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_VEC2_H__
