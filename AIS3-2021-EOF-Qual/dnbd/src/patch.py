#!/usr/bin/env python3

run_offset = 0x7C0
codesize = 0x1CB

with open('dnbd_origin.exe', 'rb') as f:
    dnbd = list(f.read())

for i in range(codesize):
    dnbd[run_offset + i] ^= 0x87

with open('dnbd.exe', 'wb') as f:
    f.write(bytes(dnbd))
