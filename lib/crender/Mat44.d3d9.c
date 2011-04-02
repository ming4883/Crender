#include "Mat44.h"

void crMat44AdjustToAPIDepthRange(CrMat44* _out)
{
	_out->m20 = (_out->m20 + _out->m30) * 0.5f;  
	_out->m21 = (_out->m21 + _out->m31) * 0.5f;
	_out->m22 = (_out->m22 + _out->m32) * 0.5f;
	_out->m23 = (_out->m23 + _out->m33) * 0.5f;
}

void crMat44AdjustToAPIProjectiveTexture(CrMat44* _out)
{
	_out->m00 = (_out->m30 + _out->m00) * 0.5f;
	_out->m01 = (_out->m31 + _out->m01) * 0.5f;
	_out->m02 = (_out->m32 + _out->m02) * 0.5f;
	_out->m03 = (_out->m33 + _out->m03) * 0.5f;

	_out->m10 = (_out->m30 - _out->m10) * 0.5f;
	_out->m11 = (_out->m31 - _out->m11) * 0.5f;
	_out->m12 = (_out->m32 - _out->m12) * 0.5f;
	_out->m13 = (_out->m33 - _out->m13) * 0.5f;
}