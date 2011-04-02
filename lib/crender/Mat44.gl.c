#include "Mat44.h"

void crMat44AdjustToAPIDepthRange(CrMat44* _out)
{
}

void crMat44AdjustToAPIProjectiveTexture(CrMat44* _out)
{
	_out->m00 = (_out->m00 + _out->m30) * 0.5f;
	_out->m01 = (_out->m01 + _out->m31) * 0.5f;
	_out->m02 = (_out->m02 + _out->m32) * 0.5f;
	_out->m03 = (_out->m03 + _out->m33) * 0.5f;

	_out->m10 = (_out->m10 + _out->m30) * 0.5f;
	_out->m11 = (_out->m11 + _out->m31) * 0.5f;
	_out->m12 = (_out->m12 + _out->m32) * 0.5f;
	_out->m13 = (_out->m13 + _out->m33) * 0.5f;
}