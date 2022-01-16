#!/usr/bin/env python3
from pwn import *

context.arch = 'amd64'

sc = asm('''
    push r12
    push r13
    mov rbp, rax
    mov rdi, rax
    call SELF
SELF:
    pop r12
    add r12, 57
    xor rsi, rsi
LOOP:
    mov al, byte ptr[rdi+rsi]
    mov r13b, byte ptr[r12+rsi]
    xor al, 174
    cmp al, r13b
    jne bye
    cmp rsi, 31
    je OK
    inc rsi
    jmp LOOP
OK:
    pop r12
    pop r13
    nop # Patch to jmp to 0x176BA (auth_ok)
    nop
    nop
    nop
    nop
bye:
    pop r12
    pop r13
    mov rbp, rdi # Do the origin logic
    test ebx, ebx
    nop # Patch to jmp to 0x1758c (jmp back to the origin logic)
    nop
    nop
    nop
    nop
''')

flag = b'FLAG{BaCkD0oooooo000oo0o0o0o0oR}'
enc_flag = [x ^ 174 for x in flag]

print(enc_flag)

sc += bytes(enc_flag)

# Generating IDA python script for patching

print("=========================")
print()

script  = 'patch = str(bytearray(' + str(list(sc)) + '))\n'
script += 'idaapi.patch_bytes(0x1c500, patch)\n'

print(script)
