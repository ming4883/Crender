#ifndef CR_VEC_H
#define CR_VEC_H

#include "cr_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef float* cr_vec;

/********** cr_vec2 **********/
CR_API void cr_vec2_set(cr_vec _out, float x, float y);

CR_API void cr_vec2_copy(cr_vec _out, cr_vec _in);

CR_API cr_bool cr_vec2_is_equal(const cr_vec a, const cr_vec b, float epsilon);

/*! return a + b */
CR_API cr_vec cr_vec2_add(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a - b */
CR_API cr_vec cr_vec2_sub(cr_vec _out, const cr_vec a, const cr_vec b);

/*! multiply a and b in a term by term  manner*/
CR_API cr_vec cr_vec2_mult(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a * b where b is a scalar*/
CR_API cr_vec cr_vec2_mult_s(cr_vec _out, const cr_vec a, float b);

/*! return a dot b */
CR_API float cr_vec2_dot(const cr_vec a, const cr_vec b);

/*! return |a|^2 */
CR_API float cr_vec2_sq_length(const cr_vec a);

/*! return |a| */
CR_API float cr_vec2_length(const cr_vec a);

/*! return |a-b| */
CR_API float cr_vec2_distance(const cr_vec a, const cr_vec b);

/*! normalize a and return |a| */
CR_API float cr_vec2_normalize(cr_vec a);

/*! return normalized copy of a */
CR_API cr_vec cr_vec2_normalized_copy(cr_vec _out, const cr_vec a);

/********** cr_vec3 **********/
CR_API void cr_vec3_set(cr_vec _out, float x, float y, float z);

CR_API void cr_vec3_copy(cr_vec _out, cr_vec _in);

CR_API cr_bool cr_vec3_is_equal(const cr_vec a, const cr_vec b, float epsilon);

/*! return a + b */
CR_API cr_vec cr_vec3_add(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a - b */
CR_API cr_vec cr_vec3_sub(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a * b */
CR_API cr_vec cr_vec3_mult(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a * b where b is a scalar*/
CR_API cr_vec cr_vec3_mult_s(cr_vec _out, const cr_vec a, float b);

/*! return a dot b */
CR_API float cr_vec3_dot(const cr_vec a, const cr_vec b);

/*! return |a|^2 */
CR_API float cr_vec3_sq_length(const cr_vec a);

/*! return |a| */
CR_API float cr_vec3_length(const cr_vec a);

/*! return |a-b| */
CR_API float cr_vec3_distance(const cr_vec a, const cr_vec b);

/*! normalize a and return |a| */
CR_API float cr_vec3_normalize(cr_vec a);

/*! return normalized copy of a */
CR_API cr_vec cr_vec3_normalized_copy(cr_vec _out, const cr_vec a);

/*! return a cross b */
CR_API cr_vec cr_vec3_cross(cr_vec _out, const cr_vec a, const cr_vec b);

/********** cr_vec4 **********/
CR_API void cr_vec4_set(cr_vec _out, float x, float y, float z, float w);

CR_API void cr_vec4_copy(cr_vec _out, cr_vec _in);

CR_API cr_bool cr_vec4_is_equal(const cr_vec a, const cr_vec b, float epsilon);

/*! return a + b */
CR_API cr_vec cr_vec4_add(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a - b */
CR_API cr_vec cr_vec4_sub(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a * b */
CR_API cr_vec cr_vec4_mult(cr_vec _out, const cr_vec a, const cr_vec b);

/*! return a * b where b is a scalar*/
CR_API cr_vec cr_vec4_mult_s(cr_vec _out, const cr_vec a, float b);

/*! return a dot b */
CR_API float cr_vec4_dot(const cr_vec a, const cr_vec b);

/*! return |a|^2 */
CR_API float cr_vec4_sq_length(const cr_vec a);

/*! return |a| */
CR_API float cr_vec4_length(const cr_vec a);

/*! return |a-b| */
CR_API float cr_vec4_distance(const cr_vec a, const cr_vec b);

/*! normalize a and return |a| */
CR_API float cr_vec4_normalize(cr_vec a);

/*! return normalized copy of a */
CR_API cr_vec cr_vec4_normalized_copy(cr_vec _out, const cr_vec a);

#ifdef __cplusplus
}
#endif

#endif	// CR_VEC_H
