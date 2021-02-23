import asyncio
import random

async def handle_echo(reader, writer):
    #data = await reader.read(100)
    #message = data.decode()
    #addr = writer.get_extra_info('peername')
    #print("Received %r from %r" % (message, addr))

    while True:
        randLat = 0.001 * random.random()
        randLon = 0.001 * random.random()
        unicode_str = "Random Coords: %.8f , %.8f" % (randLat, randLon)
        print(unicode_str)
        writer.write(b"Random Coords: %.8f , %.8f" % (randLat, randLon))
        await writer.drain()
        await asyncio.sleep(10)
    #print("Close the client socket")
    #writer.close()

loop = asyncio.get_event_loop()
coro = asyncio.start_server(handle_echo, '127.0.0.1', 8888, loop=loop)
server = loop.run_until_complete(coro)

# Serve requests until Ctrl+C is pressed
print('Serving on {}'.format(server.sockets[0].getsockname()))
try:
    loop.run_forever()
except KeyboardInterrupt:
    pass

# Close the server
server.close()
loop.run_until_complete(server.wait_closed())
loop.close()