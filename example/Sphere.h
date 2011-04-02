#ifndef __EXAMPLE_SPHERE_H__
#define __EXAMPLE_SPHERE_H__

#include "../lib/crender/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sphere
{
	CrVec3 center;
	float radius;

} Sphere;


#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_SPHERE_H__