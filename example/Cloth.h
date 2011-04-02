#ifndef __EXAMPLE_CLOTH_H__
#define __EXAMPLE_CLOTH_H__

#include "../lib/crender/Vec3.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Mesh;
struct Sphere;

typedef struct ClothConstraint
{
	size_t pIdx[2];
	float restDistance;
} ClothConstraint;

typedef struct Cloth
{
	size_t segmentCount;
	CrVec3* p;		// current positions
	CrVec3* p2;	// last positions
	CrVec3* a;		// accelerations
	CrVec3 g;		// gravity
	CrBool* fixed;		// fixed
	CrVec3* fixPos;	// fix positions
	float timeStep;	// system time step
	float damping;	// dumping due to air resistence, 0-1, 0 = no dumping

	ClothConstraint* constraints;
	size_t constraintCount;

	struct Mesh* mesh;

} Cloth;

Cloth* Cloth_new(float width, float height, const CrVec3* offset, size_t segmentCount);

void Cloth_free(Cloth* self);

void Cloth_updateMesh(Cloth* self);

void Cloth_addForceToAll(Cloth* self, const CrVec3* force);

void Cloth_verletIntegration(Cloth* self);

void Cloth_satisfyConstraints(Cloth* self);

void Cloth_collideWithSphere(Cloth* self, const struct Sphere* sphere);

void Cloth_collideWithPlane(Cloth* self, const CrVec3* normal, const CrVec3* point);

#ifdef __cplusplus
}
#endif

#endif	// __EXAMPLE_CLOTH_H__
