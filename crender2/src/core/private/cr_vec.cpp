#include "../cr_vec.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

	/********** cr_vec2 **********/
	CR_API void cr_vec2_set( cr_vec _out, float x, float y )
	{
		_out[0] = x;
		_out[1] = y;
	}

	CR_API void cr_vec2_copy( cr_vec _out, cr_vec _in )
	{
		_out[0] = _in[0];
		_out[1] = _in[1];
	}

	CR_API cr_bool cr_vec2_is_equal( const cr_vec a, const cr_vec b, float epsilon )
	{
		float ex = a[0] - b[0];
		float ey = a[1] - b[1];

		if ( ex * ex + ey * ey < epsilon )
			return CR_TRUE;

		return CR_FALSE;
	}

	CR_API cr_vec cr_vec2_add( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] + b[0];
		_out[1] = a[1] + b[1];
		return _out;
	}

	CR_API cr_vec cr_vec2_sub( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] - b[0];
		_out[1] = a[1] - b[1];
		return _out;
	}

	CR_API cr_vec cr_vec2_mult( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] * b[0];
		_out[1] = a[1] * b[1];
		return _out;
	}

	CR_API cr_vec cr_vec2_mult_s( cr_vec _out, const cr_vec a, float b )
	{
		_out[0] = a[0] * b;
		_out[1] = a[1] * b;
		return _out;
	}

	CR_API float cr_vec2_dot( const cr_vec a, const cr_vec b )
	{
		return ( a[0] * b[0] ) + ( a[1] * b[1] );
	}

	CR_API float cr_vec2_sq_length( const cr_vec a )
	{
		return cr_vec2_dot( a, a );
	}

	CR_API float cr_vec2_length( const cr_vec a )
	{
		return sqrtf( cr_vec2_dot( a, a ) );
	}

	CR_API float cr_vec2_distance( const cr_vec a, const cr_vec b )
	{
		float diff[2];
		cr_vec2_sub( diff, a, b );
		return cr_vec2_length( diff );
	}

	CR_API float cr_vec2_normalize( cr_vec a )
	{
		float len = sqrtf( cr_vec2_dot( a, a ) );
		float inv_len = 1 / ( len + 1e-5f );

		a[0] *= inv_len;
		a[1] *= inv_len;

		return len;
	}

	CR_API cr_vec cr_vec2_normalized_copy( cr_vec _out, const cr_vec a )
	{
		cr_vec2_copy( _out, a );
		cr_vec2_normalize( _out );
		return _out;
	}

	/********** cr_vec3 **********/
	CR_API void cr_vec3_set( cr_vec _out, float x, float y, float z )
	{
		_out[0] = x;
		_out[1] = y;
		_out[2] = z;
	}

	CR_API void cr_vec3_copy( cr_vec _out, cr_vec _in )
	{
		_out[0] = _in[0];
		_out[1] = _in[1];
		_out[2] = _in[2];
	}

	CR_API cr_bool cr_vec3_is_equal( const cr_vec a, const cr_vec b, float epsilon )
	{
		float ex = a[0] - b[0];
		float ey = a[1] - b[1];
		float ez = a[2] - b[2];

		if ( ex * ex + ey * ey + ez * ez < epsilon )
			return CR_TRUE;

		return CR_FALSE;
	}

	CR_API cr_vec cr_vec3_add( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] + b[0];
		_out[1] = a[1] + b[1];
		_out[2] = a[2] + b[2];
		return _out;
	}

	CR_API cr_vec cr_vec3_sub( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] - b[0];
		_out[1] = a[1] - b[1];
		_out[2] = a[2] - b[2];
		return _out;
	}

	CR_API cr_vec cr_vec3_mult( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] * b[0];
		_out[1] = a[1] * b[1];
		_out[2] = a[2] * b[2];
		return _out;
	}

	CR_API cr_vec cr_vec3_mult_s( cr_vec _out, const cr_vec a, float b )
	{
		_out[0] = a[0] * b;
		_out[1] = a[1] * b;
		_out[2] = a[2] * b;
		return _out;
	}

	CR_API float cr_vec3_dot( const cr_vec a, const cr_vec b )
	{
		return ( a[0] * b[0] ) + ( a[1] * b[1] ) + ( a[2] * b[2] );
	}

	CR_API float cr_vec3_sq_length( const cr_vec a )
	{
		return cr_vec3_dot( a, a );
	}

	CR_API float cr_vec3_length( const cr_vec a )
	{
		return sqrtf( cr_vec3_dot( a, a ) );
	}

	CR_API float cr_vec3_distance( const cr_vec a, const cr_vec b )
	{
		float diff[3];
		cr_vec3_sub( diff, a, b );
		return cr_vec3_length( diff );
	}

	CR_API float cr_vec3_normalize( cr_vec a )
	{
		float len = sqrtf( cr_vec3_dot( a, a ) );
		float inv_len = 1 / ( len + 1e-5f );

		a[0] *= inv_len;
		a[1] *= inv_len;
		a[2] *= inv_len;

		return len;
	}

	CR_API cr_vec cr_vec3_normalized_copy( cr_vec _out, const cr_vec a )
	{
		cr_vec3_copy( _out, a );
		cr_vec3_normalize( _out );
		return _out;
	}

	/* return a cross b */
	CR_API cr_vec cr_vec3_cross( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		/* reference: http://en.wikipedia.org/wiki/Cross_product#Cross_product_and_handedness */
		_out[0] = a[1] * b[2] - a[2] * b[1];
		_out[1] = a[2] * b[0] - a[0] * b[2];
		_out[2] = a[0] * b[1] - a[1] * b[0];

		return _out;
	}


	/********** cr_vec4 **********/
	CR_API void cr_vec4_set( cr_vec _out, float x, float y, float z, float w )
	{
		_out[0] = x;
		_out[1] = y;
		_out[2] = z;
		_out[3] = w;
	}

	CR_API void cr_vec4_copy( cr_vec _out, cr_vec _in )
	{
		_out[0] = _in[0];
		_out[1] = _in[1];
		_out[2] = _in[2];
		_out[3] = _in[3];
	}

	CR_API cr_bool cr_vec4_is_equal( const cr_vec a, const cr_vec b, float epsilon )
	{
		float ex = a[0] - b[0];
		float ey = a[1] - b[1];
		float ez = a[2] - b[2];
		float ew = a[3] - b[3];

		if ( ex * ex + ey * ey + ez * ez + ew * ew < epsilon )
			return CR_TRUE;

		return CR_FALSE;
	}

	CR_API cr_vec cr_vec4_add( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] + b[0];
		_out[1] = a[1] + b[1];
		_out[2] = a[2] + b[2];
		_out[3] = a[3] + b[3];
		return _out;
	}

	CR_API cr_vec cr_vec4_sub( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] - b[0];
		_out[1] = a[1] - b[1];
		_out[2] = a[2] - b[2];
		_out[3] = a[3] - b[3];
		return _out;
	}

	CR_API cr_vec cr_vec4_mult( cr_vec _out, const cr_vec a, const cr_vec b )
	{
		_out[0] = a[0] * b[0];
		_out[1] = a[1] * b[1];
		_out[2] = a[2] * b[2];
		_out[3] = a[3] * b[3];
		return _out;
	}

	CR_API cr_vec cr_vec4_mult_s( cr_vec _out, const cr_vec a, float b )
	{
		_out[0] = a[0] * b;
		_out[1] = a[1] * b;
		_out[2] = a[2] * b;
		_out[3] = a[3] * b;
		return _out;
	}

	CR_API float cr_vec4_dot( const cr_vec a, const cr_vec b )
	{
		return ( a[0] * b[0] ) + ( a[1] * b[1] ) + ( a[2] * b[2] ) + ( a[3] * b[3] );
	}

	CR_API float cr_vec4_sq_length( const cr_vec a )
	{
		return cr_vec4_dot( a, a );
	}

	CR_API float cr_vec4_length( const cr_vec a )
	{
		return sqrtf( cr_vec4_dot( a, a ) );
	}

	CR_API float cr_vec4_distance( const cr_vec a, const cr_vec b )
	{
		float diff[4];
		cr_vec4_sub( diff, a, b );
		return cr_vec4_length( diff );
	}

	CR_API float cr_vec4_normalize( cr_vec a )
	{
		float len = sqrtf( cr_vec4_dot( a, a ) );
		float inv_len = 1 / ( len + 1e-5f );

		a[0] *= inv_len;
		a[1] *= inv_len;
		a[2] *= inv_len;
		a[3] *= inv_len;

		return len;
	}

	CR_API cr_vec cr_vec4_normalized_copy( cr_vec _out, const cr_vec a )
	{
		cr_vec4_copy( _out, a );
		cr_vec4_normalize( _out );
		return _out;
	}

#ifdef __cplusplus
}
#endif
