#!/usr/bin/env python3
import socket
import time

HOST = '0.0.0.0'
PORT = 22222

def PEload(conn):
    conn.send(b'be5ac4e9\0')
    time.sleep(0.2)
    conn.send(b'd2e021ce\0')
    time.sleep(0.2)
    conn.send(b'b728d480\0')
    time.sleep(0.2)
    conn.send(b'8d744429\0')
    time.sleep(2)
    conn.send(b'3a5a5f39\0')
    time.sleep(0.2)
    conn.send(b'75207bab\0')
    time.sleep(0.2)
    conn.send(b'eae0b274\0')

def runExport(conn):
    # Order:
    #   1 -> 2
    print('1) Meow')
    print('2) Weom')

    n = int(input())

    conn.send(b'452013a2\0')
    time.sleep(0.2)

    if n == 1:
        conn.send(b'Meow\0')
        time.sleep(0.2)
        cmd(conn)
    elif n == 2:
        conn.send(b'Weom\0')
        time.sleep(0.2)

def banner():
    print('1) RUN <exe path>')
    print('2) BYE')
    print('3) EOF')
    print('4) RET')
    print('5) WRY')

def cmd(conn):
    # Order:
    #   3 -> 5 -> 4
    while True:
        banner()
        n = int(input())
        if n == 1:
            conn.send(b'RUN\0')
            time.sleep(0.2)
            conn.send(b'\\??\\C:\\Windows\\System32\\calc.exe\0')
            # conn.send(b'\\??\\C:\\Windows\\System32\\mspaint.exe\0')
            # conn.send(b'\\??\\C:\\Users\pt\\Downloads\\hello.exe\0')
            time.sleep(0.2)
        elif n == 2:
            conn.send(b'BYE\0')
            time.sleep(0.2)
            break
        elif n == 3:
            conn.send(b'EOF\0')
            time.sleep(0.2)
        elif n == 4:
            conn.send(b'RET\0')
            time.sleep(0.2)
            break
        elif n == 5:
            conn.send(b'WRY\0')
            time.sleep(0.2)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        PEload(conn)
        for _ in range(2):
            runExport(conn)
