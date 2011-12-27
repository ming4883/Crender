#ifndef CR_CONTEXT_PRIVATE_H
#define CR_CONTEXT_PRIVATE_H

#include "../cr_context.h"
#include "cr_object.h"

namespace cr
{

struct CR_API context
{
	static context* singleton;

	object* object_list;

	template<typename T> T* new_object(void) {
		T* obj = (T*)cr_mem_alloc(sizeof(T));
		obj->cstor();
		add_object(obj);
		return obj;
	}

	void add_object(object* obj);
	void del_object(object* obj);
};

}	// namespace cr

#endif	// CR_CONTEXT_PRIVATE_H
