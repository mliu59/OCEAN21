#!/usr/bin/env python3

import socket
import time

HOST = '0.0.0.0'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Waiting for client connection')
    conn, addr = s.accept()
    with conn:
        while True:
            conn.sendall(b"0,D,39.331595,-76.616543,2.2,5.2,3.2,45.1,2.1,-0.2,-0.3*")
            time.sleep(1)