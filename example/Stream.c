#include "Stream.h"

char* Stream_gets(InputStream* self, char* str, int num, void* handle)
{
	char ch;
	int cnt = 0;

	while(1) {

		if(1 != self->read(&ch, 1, 1, handle))
			return nullptr;

		str[cnt++] = ch;

		if(cnt >= num || ch == '\n')
			break;
	}

	str[cnt] = '\0';

	return str;
}
