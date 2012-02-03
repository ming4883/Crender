#include <UnitTest++.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#	include <crtdbg.h>
#endif

int main()
{
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetBreakAlloc( -1 );
#endif

	return UnitTest::RunAllTests();
}