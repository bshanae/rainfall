#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char *g_message = " - ";
const char g_space = ' ';

void p(char *target_buffer, const char *message)
{
	char temp_buffer[4104];

	puts(message);

	read(0, temp_buffer, 4096);

	*strchr(temp_buffer, 0xa) = '\0';

	strncpy(target_buffer, temp_buffer, 20);
}

void pp(char *buffer0)
{
	char buffer1[20];
	char buffer2[20];

	p(buffer2, g_message);
	p(buffer1, g_message);

	strcpy(buffer0, buffer2);
	buffer0[strlen(buffer0)] = g_space;

	strcat(buffer0, buffer1);
}

int main()
{
	char buffer0[42];

	pp(buffer0);
	puts(buffer0);

	return 0;
}