#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int language;

struct message
{
	char word1[40];
	char word2[32];
	char *language_code;
};

void greetuser(struct message message)
{
	char text[72];

	if (language == 1)
	{
		strcpy(text, "Hyv\xc3\xa4\xc3\xa4 p\xc3\xa4iv\xc3\xa4\xc3\xa4 ");
	}
	if (language == 2)
	{
		strcpy(text, "Goedemiddag! ");
	}
	if (language == 0)
	{
		strcpy(text, "Hello ");
	}

	strcat(text, message.word1);
	puts(text);
}

int main(int argc, char **argv)
{
	struct message message;

	if (argc != 3)
		return 1;

	memset(&message, 0, sizeof(struct message));

	strncpy(message.word1, argv[1], 40);
	strncpy(message.word2, argv[2], 32);

	message.language_code = getenv("LANG");
	if (message.language_code != NULL)
	{
		if (memcmp(message.language_code, "fi", 2) == 0)
			language = 1;
		if (memcmp(message.language_code, "nl", 2) == 0)
			language = 2;
	}

	greetuser(message);
	return 0;
}