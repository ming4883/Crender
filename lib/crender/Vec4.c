#include "Vec4.h"
#include "Vec3.h"

#include <math.h>

CR_API CrVec4 crVec4(float x, float y, float z, float w)
{
	CrVec4 _out;
	_out.x = x;
	_out.y = y;
	_out.z = z;
	_out.w = w;
	return _out;
}

CR_API CrVec4 crVec4FromVec3(const CrVec3* xyz, float w)
{
	CrVec4 _out;
	_out.x = xyz->x;
	_out.y = xyz->y;
	_out.z = xyz->z;
	_out.w = w;
	return _out;
}

static const CrVec4 _CrVec4_c0000 = {0, 0, 0, 0};
static const CrVec4 _CrVec4_c1000 = {1, 0, 0, 0};
static const CrVec4 _CrVec4_c0100 = {0, 1, 0, 0};
static const CrVec4 _CrVec4_c0010 = {0, 0, 1, 0};
static const CrVec4 _CrVec4_c0001 = {0, 0, 0, 1};

CR_API const CrVec4* CrVec4_c0000()
{
	return &_CrVec4_c0000;
}

CR_API const CrVec4* CrVec4_c1000()
{
	return &_CrVec4_c1000;
}

CR_API const CrVec4* CrVec4_c0100()
{
	return &_CrVec4_c0100;
}

CR_API const CrVec4* CrVec4_c0010()
{
	return &_CrVec4_c0010;
}

CR_API const CrVec4* CrVec4_c0001()
{
	return &_CrVec4_c0001;
}

CR_API CrBool crVec4IsEqual(const CrVec4* a, const CrVec4* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;
	float ew = a->w - b->w;

	if(ex * ex + ey * ey + ez * ez + ew * ew < epsilon)
		return CrTrue;

	return CrFalse;
}

CR_API CrVec4* crVec4Add(CrVec4* _out, const CrVec4* a, const CrVec4* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	_out->w = a->w + b->w;
	return _out;
}

CR_API CrVec4* crVec4Sub(CrVec4* _out, const CrVec4* a, const CrVec4* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	_out->w = a->w - b->w;
	return _out;
}

CR_API CrVec4* crVec4Mult(CrVec4* _out, const CrVec4* a, const CrVec4* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	_out->w = a->w * b->w;
	return _out;
}

CR_API CrVec4* crVec4MultS(CrVec4* _out, const CrVec4* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	_out->w = a->w * b;
	return _out;
}

CR_API float crVec4Dot(const CrVec4* a, const CrVec4* b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z) + (a->w * b->w);
}

CR_API float crVec4SqLength(const CrVec4* a)
{
	return crVec4Dot(a, a);
}

CR_API float crVec4Length(const CrVec4* a)
{
	return sqrtf(crVec4SqLength(a));
}

CR_API float crVec4Distance(const CrVec4* a, const CrVec4* b)
{
	CrVec4 diff;
	crVec4Sub(&diff, a, b);
	return crVec4Length(&diff);
}

CR_API float crVec4Normalize(CrVec4* a)
{
	float len = crVec4Length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
		a->z *= inv_len;
		a->w *= inv_len;
	}

	return len;
}

CR_API CrVec4 crVec4NormalizedCopy(const CrVec4* a)
{
	CrVec4 _out = *a;
	crVec4Normalize(&_out);
	return _out;
}
