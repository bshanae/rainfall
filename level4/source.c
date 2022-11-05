#include <stdio.h>
#include <stdlib.h>

int m;

void p(const char *str)
{
	printf(str);
}

void n()
{
	char buffer[512];

	fgets(buffer, 512, stdin);
	p(buffer);

	if (m != 0x1025544)
		return;

	system("/bin/cat /home/user/level5/.pass");
}

int main()
{
	n();
	return 0;
}
