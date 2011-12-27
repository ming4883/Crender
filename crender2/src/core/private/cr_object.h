#ifndef CR_OBJECT_PRIVATE_H
#define CR_OBJECT_PRIVATE_H

namespace cr
{

struct object;
typedef void (*object_dstor_func) (object* obj);

struct object
{
	object_dstor_func dstor_func;
	object *next, *prev;

	unsigned long ref_cnt;

	void cstor()
	{
		dstor_func = nullptr;
		next = prev = nullptr;
		ref_cnt = 1;
	}
};

}	// namespace cr

#endif	// CR_OBJECT_PRIVATE_H
