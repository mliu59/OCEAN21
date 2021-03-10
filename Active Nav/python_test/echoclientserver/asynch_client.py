import asyncio


async def tcp_echo_client(loop):
    reader, writer = await asyncio.open_connection('127.0.0.1', 8888,
                                                   loop=loop)
    while True:
        data = await reader.read(255)
        if data:
            print('Received: %r' % data.decode())


loop = asyncio.get_event_loop()
loop.run_until_complete(tcp_echo_client(loop))