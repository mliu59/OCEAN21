import socket
import time
import random

HOST = '127.0.0.1'
PORT = 11111
encoding = 'utf-8'

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected via ', addr)
        while True:
            randLat = 43 + random.random()
            randLon = 43 + random.random()
            unicode_str = "%.8f , %.8f" % (randLat, randLon)
            print(unicode_str)
            s.sendall(unicode_str.encode())
            time.sleep(5)