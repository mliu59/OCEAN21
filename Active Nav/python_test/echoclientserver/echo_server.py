#!/usr/bin/env python3

import socket
import time
import queue

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

queue = [[39.363303, -76.339240], [39.362940, -76.339271], [39.362942, -76.339605], [39.363249, -76.339550]]

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Waiting for client connection')
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        
        conn.sendall(b"arm")
        time.sleep(20)
        counter = 0
        while counter < 12:
            coord = queue.pop(0)
            conn.sendall(b"goto,%.8f,%.8f" % (coord[0], coord[1]))
            queue.append(coord)
            counter += 1
            time.sleep(20)
            
        conn.sendall(b"disarm")
        time.sleep(20)
        conn.sendall(b"quit")