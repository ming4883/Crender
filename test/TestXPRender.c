
#include "../lib/cutest/CuTest.h"

#include <stdio.h>
#include "../lib/crender/Platform.h"


#if defined(CR_VC)
#	define RDTSC(low, high)	\
	__asm rdtsc				\
	__asm mov low, eax		\
	__asm mov high, edx
#elif defined(CR_GCC)
#	define RDTSC(low, high)	\
	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#endif

// mftbu in PowerPC: http://lists.apple.com/archives/mac-games-dev/2002/May/msg00244.html
uint64_t rdtsc()
{
	uint32_t l, h;
	RDTSC(l, h);
	return (((uint64_t)h) << 32) + l;
}

#include "../lib/crender/NvpParser.h"

void TestNvpParser(CuTest *tc)
{
	const char* str = "name1=value1;name2=\"value 2\";";
	const char* name;
	const char* value;
	CrBool hasNext;
	CrNvpParser* nvp = crNvpParserAlloc();
	crNvpParserInit(nvp, str);

	hasNext = crNvpParserNext(nvp, &name, &value);

	CuAssertIntEquals(tc, CrTrue, hasNext);
	CuAssertStrEquals(tc, "name1", name);
	CuAssertStrEquals(tc, "value1", value);

	hasNext = crNvpParserNext(nvp, &name, &value);

	CuAssertIntEquals(tc, CrTrue, hasNext);
	CuAssertStrEquals(tc, "name2", name);
	CuAssertStrEquals(tc, "value 2", value);

	hasNext = crNvpParserNext(nvp, &name, &value);

	CuAssertIntEquals(tc, CrFalse, hasNext);

	crNvpParserFree(nvp);
}

#include "../lib/crender/StrHash.h"

void TestStrHash(CuTest *tc)
{
	const char* s1 = "u_worldViewMtx";
	const char* s2 = "u_worldViewProjMtx";

	CuAssertTrue(tc, CrHash("u_worldViewMtx") != CrHash("u_worldViewProjMtx"));
	CuAssertTrue(tc, CrHash(s1) != CrHash(s2));

	CuAssertTrue(tc, CrHash("u_worldViewMtx") == CrHash(s1));
	CuAssertTrue(tc, CrHash("u_worldViewProjMtx") == CrHash(s2));
}

typedef struct Foo
{
	size_t a, b, c;
} Foo;

void TestStrHashStruct(CuTest *tc)
{
	Foo f1a = {0x000000a7, 0x00000002, 0x00000051};
	Foo f1b = {0x00000145, 0x00000000, 0x0000013a};
	Foo f2a = {0x00000115, 0x00000002, 0x000000ff};
	Foo f2b = {0x0000012c, 0x00000002, 0x00000127};
	Foo f3a = {0x00000102, 0x00000000, 0x000000fd};
	Foo f3b = {0x000001f5, 0x00000003, 0x000001ee};

	CuAssertTrue(tc, CrHashStruct(&f1a, sizeof(Foo)) != CrHashStruct(&f1b, sizeof(Foo)));
	CuAssertTrue(tc, CrHashStruct(&f2a, sizeof(Foo)) != CrHashStruct(&f2b, sizeof(Foo)));
	CuAssertTrue(tc, CrHashStruct(&f3a, sizeof(Foo)) != CrHashStruct(&f3b, sizeof(Foo)));
}

int gResult;	// this avoid compile optimization

void TestStrHashPerformance(CuTest *tc)
{
	uint64_t startTime;
	int cnt = 100000;
	int itCnt = 100;
	int i;
	int it;

	uint64_t t1 = 0;
	uint64_t t2 = 0;

	for(it = 0 ;it < itCnt; ++it) {
		startTime = rdtsc();
		for(i=0; i<cnt; ++i) {
			CrHashCode c1 = CrHash("u_worldViewMtx");
			CrHashCode c2 = CrHash("u_worldViewProjMtx");
			int result = c1 > c2 ? 1 : -1;
			gResult += result;
		}
		t1 += rdtsc() - startTime;
	}

	for(it = 0 ;it < itCnt; ++it) {
		startTime = rdtsc();
		for(i=0; i<cnt; ++i) {
			int result = strcmp("u_worldViewMtx", "u_worldViewProjMtx");
			gResult += result;
		}

		t2 += rdtsc() - startTime;
	}

	printf("CrStrHash done in %d ticks\n", t1 / itCnt);
	printf("strcmp     done in %d ticks\n", t2 / itCnt);
}

CuSuite* CrGetSuite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, TestNvpParser);
	SUITE_ADD_TEST(suite, TestStrHash);
	SUITE_ADD_TEST(suite, TestStrHashStruct);
	SUITE_ADD_TEST(suite, TestStrHashPerformance);
	return suite;
}

void RunAllTests()
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, CrGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(int argc, char** argv)
{
	RunAllTests();
	
	return 0;
}