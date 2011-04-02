#include "Vec2.h"

#include <math.h>

CrVec2 crVec2(float x, float y)
{
	CrVec2 _out;
	_out.x = x;
	_out.y = y;
	return _out;
}

static const CrVec2 _CrVec2_c00 = {0, 0};
static const CrVec2 _CrVec2_c10 = {1, 0};
static const CrVec2 _CrVec2_c01 = {0, 1};

const CrVec2* CrVec2_c00()
{
	return &_CrVec2_c00;
}

const CrVec2* CrVec2_c10()
{
	return &_CrVec2_c10;
}

const CrVec2* CrVec2_c01()
{
	return &_CrVec2_c01;
}


void crVec2_set(CrVec2* _out, float x, float y)
{
	if(nullptr == _out)
		return;

	_out->x = x;
	_out->y = y;
}

CrBool crVec2IsEqual(const CrVec2* a, const CrVec2* b, float epsilon)
{
	float ex = a->x - b->x;
	float ey = a->y - b->y;

	if(ex * ex + ey * ey < epsilon)
		return CrTrue;

	return CrFalse;
}

CrVec2* crVec2Add(CrVec2* _out, const CrVec2* a, const CrVec2* b)
{
	_out->x = a->x + b->x;
	_out->y = a->y + b->y;
	return _out;
}

CrVec2* crVec2Sub(CrVec2* _out, const CrVec2* a, const CrVec2* b)
{
	_out->x = a->x - b->x;
	_out->y = a->y - b->y;
	return _out;
}

CrVec2* crVec2Mult(CrVec2* _out, const CrVec2* a, const CrVec2* b)
{
	_out->x = a->x * b->x;
	_out->y = a->y * b->y;
	return _out;
}

CrVec2* crVec2MultS(CrVec2* _out, const CrVec2* a, float b)
{
	_out->x = a->x * b;
	_out->y = a->y * b;
	return _out;
}

float crVec2Dot(const CrVec2* a, const CrVec2* b)
{
	return (a->x * b->x) + (a->y * b->y);
}

float crVec2SqLength(const CrVec2* a)
{
	return crVec2Dot(a, a);
}

float crVec2Length(const CrVec2* a)
{
	return sqrtf(crVec2SqLength(a));
}

float crVec2Distance(const CrVec2* a, const CrVec2* b)
{
	CrVec2 diff;
	crVec2Sub(&diff, a, b);
	return crVec2Length(&diff);
}

float crVec2Normalize(CrVec2* a)
{
	float len = crVec2Length(a);
	
	if(len > 1e-5f || len < -1e-5f)
	{
		float inv_len = 1 / len;

		a->x *= inv_len;
		a->y *= inv_len;
	}

	return len;
}

CrVec2 crVec2NormalizedCopy(const CrVec2* a)
{
	CrVec2 _out = *a;
	crVec2Normalize(&_out);
	return _out;
}
