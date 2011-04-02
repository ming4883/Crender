#include "Vec3.h"
#include "Vec2.h"
#include <math.h>

CrVec3 crVec3(float x, float y, float z)
{
	CrVec3 _out;
	_out.x = x;
	_out.y = y;
	_out.z = z;
	return _out;
}

CrVec3 crVec3FromVec2(const CrVec2* xy, float z)
{
	CrVec3 _out;
	_out.x = xy->x;
	_out.y = xy->y;
	_out.z = z;
	return _out;
}

static const CrVec3 _CrVec3_c000 = {0, 0, 0};
static const CrVec3 _CrVec3_c100 = {1, 0, 0};
static const CrVec3 _CrVec3_c010 = {0, 1, 0};
static const CrVec3 _CrVec3_c001 = {0, 0, 1};

const CrVec3* CrVec3_c000()
{
	return &_CrVec3_c000;
}

const CrVec3* CrVec3_c100()
{
	return &_CrVec3_c100;
}

const CrVec3* CrVec3_c010()
{
	return &_CrVec3_c010;
}

const CrVec3* CrVec3_c001()
{
	return &_CrVec3_c001;
}

void crVec3Set(CrVec3* _out, float x, float y, float z)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
	_out->z = z;
}

CrBool crVec3IsEqual(const CrVec3* a, const CrVec3* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;
	float ez = a->z - b->z;

	if(ex * ex + ey * ey + ez * ez < epsilon)
		return CrTrue;

	return CrFalse;
}

CrVec3* crVec3Add(CrVec3* _out, const CrVec3* a, const CrVec3* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	_out->z = a->z + b->z;
	return _out;
}

CrVec3* crVec3Sub(CrVec3* _out, const CrVec3* a, const CrVec3* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	_out->z = a->z - b->z;
	return _out;
}

CrVec3* crVec3Mult(CrVec3* _out, const CrVec3* a, const CrVec3* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	_out->z = a->z * b->z;
	return _out;
}

CrVec3* crVec3MultS(CrVec3* _out, const CrVec3* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	_out->z = a->z * b;
	return _out;
}

float crVec3Dot(const CrVec3* a, const CrVec3* b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

float crVec3SqLength(const CrVec3* a)
{
	return crVec3Dot(a, a);
}

float crVec3Length(const CrVec3* a)
{
	return sqrtf(crVec3SqLength(a));
}

float crVec3Distance(const CrVec3* a, const CrVec3* b)
{
	CrVec3 diff;
	return crVec3Length(crVec3Sub(&diff, a, b));
}

float crVec3Normalize(CrVec3* a)
{
	float len = crVec3Length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
		a->z *= inv_len;
	}

	return len;
}

CrVec3 crVec3NormalizedCopy(const CrVec3* a)
{
	CrVec3 _out = *a;
	crVec3Normalize(&_out);
	return _out;
}

CrVec3* crVec3Cross(CrVec3* _out, const CrVec3* a, const CrVec3* b)
{
	/* reference: http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness */
	_out->x = a->y * b->z - a->z * b->y;
	_out->y = a->z * b->x - a->x * b->z;
	_out->z = a->x * b->y - a->y * b->x;

	return _out;
}
