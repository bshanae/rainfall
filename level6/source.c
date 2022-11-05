#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void n()
{
	system("/bin/cat /home/user/level7/.pass");
}

void m()
{
	puts("Nope");
}

typedef void (*t_func)();

int main(int argc, char **argv)
{
	char *str;
	t_func *func;

	str = malloc(64);
	func = malloc(sizeof(t_func));

	*func = m;
	strcpy(str, argv[1]);

	(*func)();

	return 0;
}
