import asyncio
import random


async def handle_client():
    reader, writer = await asyncio.open_connection(
        '127.0.0.1', 11111)

    while True:
        #randLat = 43 + random.random()
        #randLon = 43 + random.random()
        #unicode_str = "Random Coords: %.8f , %.8f" % (randLat, randLon)
        #print(unicode_str)
        #writer.write(unicode_str.encode())
        #await writer.drain()
        await asyncio.sleep(2)
    #writer.close()

asyncio.get_event_loop().run_until_complete(handle_client())