import socket
import time
import random

encoding = 'utf-8'

while True:
    randLat = random.randint(40, 43) + random.random()
    randLon = random.randint(40, 43) + random.random()
    unicode_str = "Random Coords: %.8f , %.8f" % (randLat, randLon)
    print(unicode_str)
    print(unicode_str.encode(encoding))
    time.sleep(5)