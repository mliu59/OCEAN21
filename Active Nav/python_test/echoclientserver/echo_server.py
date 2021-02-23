#!/usr/bin/env python3

import socket
import time
import random

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print('Waiting for client connection')
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            randLat = 43 + random.random()
            randLon = 43 + random.random()
            unicode_str = "Random Coords: %.8f , %.8f" % (randLat, randLon)
            print(unicode_str)
            conn.sendall(b"Random Coords: %.8f , %.8f" % (randLat, randLon))
            time.sleep(15)
