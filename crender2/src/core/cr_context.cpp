#include "private/cr_context.h"
#include "uthash/utlist.h"

namespace cr
{

context* context::singleton = nullptr;

object::object( context* ctx )
{
	ctx->add_object( this );
	ref_cnt = 1;
}

object::~object( void )
{
	_context->rmv_object( this );
}

void context::add_object( object* obj )
{
	obj->_context = this;
	obj->next = obj->prev = nullptr;
	DL_APPEND( object_list, obj );
}

void context::rmv_object( object* obj )
{
	DL_DELETE( object_list, obj );
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

	CR_API void cr_context_initialize( void )
	{
		cr::context* self = ( cr::context* )cr_mem_alloc( sizeof( cr::context ) );
		self->object_list = nullptr;

		cr::context::singleton = self;
	}

	CR_API void cr_context_finalize( void )
	{
		if ( nullptr == cr::context::singleton )
			return;

		cr::context* self = cr::context::singleton;

		if ( nullptr != self->object_list )
		{
			// delete all objects
			cr::object* curr = nullptr, * temp = nullptr;
			DL_FOREACH_SAFE( self->object_list, curr, temp )
			{
				//self->del_object( curr );
				delete curr;
			}
		}

		cr_mem_free( self );

		cr::context::singleton = nullptr;
	}

	CR_API void cr_retain( cr_object obj )
	{
		CR_ASSERT( obj );
		cr::object* self = ( cr::object* )obj;
		++self->ref_cnt;
	}

	CR_API void cr_release( cr_object obj )
	{
		CR_ASSERT( obj );
		cr::object* self = ( cr::object* )obj;

		--self->ref_cnt;

		if ( 0 == self->ref_cnt )
			delete self;
	}

	CR_API cr_uint32 cr_ref_count_of( cr_object obj )
	{
		CR_ASSERT( obj );
		cr::object* self = ( cr::object* )obj;
		return self->ref_cnt;
	}

#ifdef __cplusplus
}
#endif
