#include "NvpParser.h"
#include "Mem.h"

CR_API CrNvpParser* crNvpParserAlloc()
{
	CrNvpParser* self = crMem()->alloc(sizeof(CrNvpParser), "CrNvpParser");
	memset(self, 0, sizeof(CrNvpParser));
	return self;
}

CR_API void crNvpParserFree(CrNvpParser* self)
{
	crMem()->free(self->mStr, "CrNvpParser");
	crMem()->free(self, "CrNvpParser");
}

CR_API void crNvpParserInit(CrNvpParser* self, const char* str)
{
	self->mStr = self->mPos = crMem()->alloc(strlen(str), "CrNvpParser");
	memcpy(self->mStr, str, strlen(str));
}

CrBool advancePos(char** pos)
{
	if(**pos != '\0') {
		++(*pos);
		return CrTrue;
	}
	return CrFalse;
}

static void skipSeps(char** pos)
{
	static const char cSeps[] = ",; \t\n\r";
	static const size_t cCount = sizeof(cSeps) - 1;
	size_t i;

	for(i=cCount; i--;) {
		if(**pos != cSeps[i])
			continue;
		i = cCount;
		if(!advancePos(pos))
			return;
	}
}

void skipNonSeps(char** pos)
{
	static const char cSeps[] = "=,; \t\n\r";
	static const size_t cCount = sizeof(cSeps) - 1;

	size_t i;
	do {
		for(i=cCount; i--;) {
			if(**pos == cSeps[i])
				return;
		}
	} while(advancePos(pos));
}

CR_API CrBool crNvpParserNext(CrNvpParser* self, const char** name, const char** value)
{
	static const char cQuots[] = "'\"";
	char* name_;

	// Get the name
	skipSeps(&self->mPos);
	name_ = self->mPos;
	skipNonSeps(&self->mPos);

	if(*self->mPos != '=' && advancePos(&self->mPos)) {
		*(self->mPos-1) = '\0';
		skipSeps(&self->mPos);
	}

	// Should be '='
	if(*self->mPos != '=')
		return CrFalse;
	*(self->mPos++) = '\0';

	// Get the value
	skipSeps(&self->mPos);
	// Get quoted string
	if(*self->mPos == cQuots[0] || *self->mPos == cQuots[1]) {
		char quot = *self->mPos;
		*value = self->mPos + 1;
		while(advancePos(&self->mPos) && *self->mPos != quot);
	} else {
		*value = self->mPos;
		skipNonSeps(&self->mPos);
	}

	if(advancePos(&self->mPos))
		*(self->mPos-1) = '\0';

	*name = name_;
	return CrTrue;
}
