#ifndef __CRENDER_VEC3_H__
#define __CRENDER_VEC3_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrVec2;

typedef struct CrVec3
{
	union
	{
		struct {
			float x;
			float y;
			float z;
		};
		float v[3];
	};
} CrVec3;

CR_API CrVec3 crVec3(float x, float y, float z);

CR_API CrVec3 crVec3FromVec2(const struct CrVec2* xy, float z);

CR_API const CrVec3* CrVec3_c000();
CR_API const CrVec3* CrVec3_c100();
CR_API const CrVec3* CrVec3_c010();
CR_API const CrVec3* CrVec3_c001();

#define crVec3Set(_out, x, y, z) {_out.x = x; _out.y = y; _out.z = z;}

CR_API CrBool crVec3IsEqual(const CrVec3* a, const CrVec3* b, float epsilon);

/* return a + b */
CR_API CrVec3* crVec3Add(CrVec3* _out, const CrVec3* a, const CrVec3* b);

/* return a - b */
CR_API CrVec3* crVec3Sub(CrVec3* _out, const CrVec3* a, const CrVec3* b);

/* return term by term a * b */
CR_API CrVec3* crVec3Mult(CrVec3* _out, const CrVec3* a, const CrVec3* b);

/* return a * b */
CR_API CrVec3* crVec3MultS(CrVec3* _out, const CrVec3* a, float b);

/* return a dot b */
CR_API float crVec3Dot(const CrVec3* a, const CrVec3* b);

/* return |a|^2 */
CR_API float crVec3SqLength(const CrVec3* a);

/* return |a| */
CR_API float crVec3Length(const CrVec3* a);

/* return |a-b| */
CR_API float crVec3Distance(const CrVec3* a, const CrVec3* b);

/* normalize a and return |a| */
CR_API float crVec3Normalize(CrVec3* a);

/* return normalized copy of a */
CR_API CrVec3 crVec3NormalizedCopy(const CrVec3* a);

/* return a cross b */
CR_API CrVec3* crVec3Cross(CrVec3* _out, const CrVec3* a, const CrVec3* b);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_VEC3_H__
