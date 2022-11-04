#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void p()
{
	void* return_ptr;
	char buffer[64];

	fflush(stdin);
	gets(buffer);

	return_ptr = __builtin_return_address(0);

	if (((int32_t)return_ptr & 0xb0000000) == 0xb0000000)
	{
		printf("(%p)\n", return_ptr);
		exit(1);
	}

	puts(buffer);
	strdup(buffer);
}

int main()
{
	p();
	return 0;
}