#include "Cloth.h"

#include "Mesh.h"
#include "Sphere.h"

#include "../lib/crender/Vec2.h"

#include <stdio.h>

void Cloth_makeConstraint(Cloth* self, size_t x0, size_t y0, size_t x1, size_t y1)
{
	size_t idx0 = y0 * self->segmentCount + x0;
	size_t idx1 = y1 * self->segmentCount + x1;

	ClothConstraint* constraint = &self->constraints[self->constraintCount];
	constraint->pIdx[0] = idx0;
	constraint->pIdx[1] = idx1;
	constraint->restDistance = crVec3Distance(&self->p[idx0], &self->p[idx1]);

	++self->constraintCount;
}

Cloth* Cloth_new(float width, float height, const CrVec3* offset, size_t segmentCount)
{
	size_t r, c;
	unsigned short* mapped = nullptr;
	CrVec2* uv = nullptr;

	Cloth* self = (Cloth*)malloc(sizeof(Cloth));
	self->segmentCount = segmentCount;
	self->g = crVec3(0, -1, 0);
	self->timeStep = 0;
	self->damping = 0;

	self->p = (CrVec3*)malloc(sizeof(CrVec3) * segmentCount * segmentCount);
	self->p2 = (CrVec3*)malloc(sizeof(CrVec3) * segmentCount * segmentCount);
	self->a = (CrVec3*)malloc(sizeof(CrVec3) * segmentCount * segmentCount);
	self->fixPos = (CrVec3*)malloc(sizeof(CrVec3) * segmentCount * segmentCount);
	self->fixed = (CrBool*)malloc(sizeof(CrBool) * segmentCount * segmentCount);

	self->mesh = meshAlloc();
	meshInit(self->mesh, segmentCount * segmentCount, (segmentCount-1) * (segmentCount-1) * 6);
	
	mapped = (unsigned short*)self->mesh->index.buffer;
	uv = (CrVec2*)self->mesh->texcoord[0].buffer;

	for(r=0; r<segmentCount-1; ++r) {
		for(c=0; c<segmentCount-1; ++c) {
			unsigned short p0 = (unsigned short)(r * segmentCount + c);
			unsigned short p1 = (unsigned short)((r+1) * segmentCount + c);
			unsigned short p2 = (unsigned short)(r * segmentCount + (c+1));
			unsigned short p3 = (unsigned short)((r+1) * segmentCount + (c+1));

			(*mapped++) = p0;
			(*mapped++) = p1;
			(*mapped++) = p2;

			(*mapped++) = p3;
			(*mapped++) = p2;
			(*mapped++) = p1;
		}
	}

	for(r=0; r<segmentCount; ++r) {
		float y = offset->v[2] - height * (float)r / segmentCount;
		for(c=0; c<segmentCount; ++c) {
			size_t i = r * segmentCount + c;
			float x = offset->v[0] + width * (float)c / segmentCount;
			CrVec3 p = crVec3(x, offset->v[1], y);

			self->p[i] = p;
			self->p2[i] = p;
			self->fixPos[i] = p;
			self->fixed[i] = CrFalse;

			self->a[i].x = 0;
			self->a[i].y = 0;
			self->a[i].z = 0;

			uv[i].x = width  * (float)c / segmentCount;
			uv[i].y = height * (float)r / segmentCount;
		}
	}

	self->fixed[0] = CrTrue;
	self->fixed[segmentCount-1] = CrTrue;

	// setup constraints
	self->constraints = (ClothConstraint*)malloc(sizeof(ClothConstraint) * self->segmentCount * self->segmentCount * 8);
	self->constraintCount = 0;

	for(r=0; r<segmentCount; ++r) {
		for(c=0; c<segmentCount; ++c) {
			if(r+1 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+1, c);

			if(c+1 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+1);

			if(r+1 < segmentCount && c+1 < segmentCount) {
				Cloth_makeConstraint(self, r, c, r+1, c+1);
				Cloth_makeConstraint(self, r+1, c, r, c+1);
			}
			
			if(r+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r+2, c);

			if(c+2 < segmentCount)
				Cloth_makeConstraint(self, r, c, r, c+2);

			if(r+2 < segmentCount && c+2 < segmentCount) {
				Cloth_makeConstraint(self, r, c, r+2, c+2);
				Cloth_makeConstraint(self, r+2, c, r, c+2);
			}
		}
	}

	return self;
}

void Cloth_free(Cloth* self)
{
	meshFree(self->mesh);
	free(self->p);
	free(self->p2);
	free(self->a);
	free(self->constraints);
	free(self->fixed);
	free(self->fixPos);
	free(self);
}

void Cloth_addForceToAll(Cloth* self, const CrVec3* force)
{
	size_t i, cnt = self->segmentCount * self->segmentCount;
	for(i = 0; i < cnt; ++i)
	{
		CrVec3* a = &self->a[i];
		crVec3Add(a, a, (CrVec3*)force);
	}
}

void Cloth_updateMesh(Cloth* self)
{
	size_t r, c;

	CrVec3* normals = (CrVec3*)self->mesh->normal.buffer;

	for(r = 0; r < self->segmentCount; ++r) {
		for(c = 0; c < self->segmentCount; ++c) {
			CrVec3 n = *CrVec3_c000();
			int cnt = 0;

			CrVec3* p1, *p2, *p3;
			CrVec3 v1, v2, n2;
			p1 = &self->p[r * self->segmentCount + c];

			if(r>0 && c>0) {
				p2 = &self->p[(r) * self->segmentCount + (c-1)];
				p3 = &self->p[(r-1) * self->segmentCount + c];
				
				crVec3Sub(&v1, p2, p1);
				crVec3Sub(&v2, p3, p1);
				crVec3Cross(&n2, &v1, &v2);
				crVec3Normalize(&n2);
				crVec3Add(&n, &n, &n2);
				++cnt;
			}

			if(r>0 && c<(self->segmentCount-1)) {
				p2 = &self->p[(r-1) * self->segmentCount + c];
				p3 = &self->p[(r) * self->segmentCount + (c+1)];
				
				crVec3Sub(&v1, p2, p1);
				crVec3Sub(&v2, p3, p1);
				crVec3Cross(&n2, &v1, &v2);
				crVec3Normalize(&n2);
				crVec3Add(&n, &n, &n2);
				++cnt;
			}

			if(r<(self->segmentCount-1) && c<(self->segmentCount-1)) {
				p2 = &self->p[(r) * self->segmentCount + (c+1)];
				p3 = &self->p[(r+1) * self->segmentCount + c];
				
				crVec3Sub(&v1, p2, p1);
				crVec3Sub(&v2, p3, p1);
				crVec3Cross(&n2, &v1, &v2);
				crVec3Normalize(&n2);
				crVec3Add(&n, &n, &n2);
				++cnt;
			}

			if(r<(self->segmentCount-1) && c>0) {
				p2 = &self->p[(r+1) * self->segmentCount + c];
				p3 = &self->p[(r) * self->segmentCount + (c-1)];
				
				crVec3Sub(&v1, p2, p1);
				crVec3Sub(&v2, p3, p1);
				crVec3Cross(&n2, &v1, &v2);
				crVec3Normalize(&n2);
				crVec3Add(&n, &n, &n2);
				++cnt;
			}
		
			if(cnt > 0)
				crVec3Normalize(&n);
			n.x *= -1;
			n.y *= -1;
			n.z *= -1;
			(*normals++) = n;
		}
	}
	
	memcpy(self->mesh->vertex.buffer, self->p, self->mesh->vertex.sizeInBytes);

	meshCommit(self->mesh);
}

void Cloth_verletIntegration(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	float t2 = self->timeStep * self->timeStep;

	// Verlet Integration
	for(i = 0; i < cnt; ++i) {
		CrVec3* x = &self->p[i];
		CrVec3* oldx = &self->p2[i];
		CrVec3* a = &self->a[i];

		CrVec3 tmp = *x;
		CrVec3 dx;
		CrVec3 da;
		crVec3MultS(&da, a, t2);
		crVec3Sub(&dx, x, oldx);
		crVec3MultS(&dx, &dx, 1-self->damping);
		crVec3Add(&dx, &dx, &da);

		crVec3Add(x, x, &dx);

		*a = *CrVec3_c000();
		*oldx = tmp;
	}
}

void Cloth_satisfyConstraints(Cloth* self)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i) {
		if(CrTrue == self->fixed[i])
			self->p[i] = self->fixPos[i];
	}

	for(i=0; i<self->constraintCount; ++i) {
		ClothConstraint* c = &self->constraints[i];
		CrVec3* x1 = &self->p[c->pIdx[0]];
		CrVec3* x2 = &self->p[c->pIdx[1]];
		CrVec3 delta;
		float scale;
		crVec3Sub(&delta, x2, x1);
		
		scale = (1 - c->restDistance / crVec3Length(&delta)) * 0.5f;
		crVec3MultS(&delta, &delta, scale);

		crVec3Add(x1, x1, &delta);
		crVec3Sub(x2, x2, &delta);
	}
}

const float collisionEpsilon = 1e-1f;

void Cloth_collideWithSphere(Cloth* self, const Sphere* sphere)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;

	for(i = 0; i < cnt; ++i) {
		CrVec3* x = &self->p[i];
		CrVec3 d;
		float l;

		crVec3Sub(&d, x, &sphere->center);
		l = crVec3Length(&d);

		if(l < sphere->radius) {
			crVec3MultS(&d, &d, (sphere->radius - (l - collisionEpsilon)) / (l - collisionEpsilon));
			crVec3Add(x, x, &d);
		}
	}
}

void Cloth_collideWithPlane(Cloth* self, const CrVec3* normal, const CrVec3* point)
{
	size_t i;
	size_t cnt = self->segmentCount * self->segmentCount;
	float d = -crVec3Dot(normal, point);
	float e = collisionEpsilon * 1e-1f;

	for(i = 0; i < cnt; ++i) {
		CrVec3* x = &self->p[i];
		float l = crVec3Dot((CrVec3*)normal, x) + d;
		if(l < e) {
			CrVec3 dx;
			crVec3MultS(&dx, normal, -(l - e));
			crVec3Add(x, x, &dx);
		}
	}
}