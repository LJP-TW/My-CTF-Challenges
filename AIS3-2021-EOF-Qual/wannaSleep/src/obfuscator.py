#!/usr/bin/env python3

def obfuscator(s, w=False):
    for i in range(len(s)):
        s[i] = s[i] ^ 0x87 ^ i
    if w == False:
        s.append(0)
        return s
    else:
        r = []
        for b in s:
            r.append(b)
            r.append(0xe8)
        r.append(0)
        return r

# print(obfuscator(list(b'ntdll.dll'), w=True))
# print(obfuscator(list(b'RtlAllocateHeap')))
# print(obfuscator(list(b'kernel32.dll'), w=True))
# print(obfuscator(list(b'LoadLibraryA')))
# print(obfuscator(list(b'HeapCreate')))
# print(obfuscator(list(b'CreateFileA')))
# print(obfuscator(list(b'GetFileSizeEx')))
# print(obfuscator(list(b'ReadFile')))
# print(obfuscator(list(b'WriteFile')))
# print(obfuscator(list(b'CloseHandle')))
# print(obfuscator(list(b'shlwapi.dll'), w=True))
# print(obfuscator(list(b'shlwapi.dll')))
print(obfuscator(list(b'PathFileExistsA')))
