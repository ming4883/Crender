#ifndef __CRENDER_VEC4_H__
#define __CRENDER_VEC4_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrVec3;

typedef struct CrVec4
{
	union
	{
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		float v[4];
	};
} CrVec4;

CR_API CrVec4 crVec4(float x, float y, float z, float w);
CR_API CrVec4 crVec4FromVec3(const struct CrVec3* xyz, float w);

CR_API const CrVec4* CrVec4_c0000();
CR_API const CrVec4* CrVec4_c1000();
CR_API const CrVec4* CrVec4_c0100();
CR_API const CrVec4* CrVec4_c0010();
CR_API const CrVec4* CrVec4_c0001();

#define crVec4Set(_out, x, y, z, w) {_out.x = x; _out.y = y; _out.z = z; _out.w = w;}

CR_API CrBool crVec4IsEqual(const CrVec4* a, const CrVec4* b, float epsilon);

/* return a + b */
CR_API CrVec4* crVec4Add(CrVec4* _out, const CrVec4* a, const CrVec4* b);

/* return a - b */
CR_API CrVec4* crVec4Sub(CrVec4* _out, const CrVec4* a, const CrVec4* b);

/* return a * b */
CR_API CrVec4* crVec4Mult(CrVec4* _out, const CrVec4* a, const CrVec4* b);

/* return a * b */
CR_API CrVec4* crVec4MultS(CrVec4* _out, const CrVec4* a, float b);

/* return a dot b */
CR_API float crVec4Dot(const CrVec4* a, const CrVec4* b);

/* return |a|^2 */
CR_API float crVec4SqLength(const CrVec4* a);

/* return |a| */
CR_API float crVec4Length(const CrVec4* a);

/* return |a-b| */
CR_API float crVec4Distance(const CrVec4* a, const CrVec4* b);

/* normalize a and return |a| */
CR_API float crVec4Normalize(CrVec4* a);

/* return normalized copy of a */
CR_API CrVec4 crVec4NormalizedCopy(const CrVec4* a);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_VEC4_H__
