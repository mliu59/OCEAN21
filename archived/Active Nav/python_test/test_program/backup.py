import asyncio
from mavsdk import System
from mavsdk.follow_me import (Config, FollowMeError, TargetLocation)
from queue import Queue

HOST = '127.0.0.1'
PORT = 11111
encoding = 'utf8'

default_height = 8.0 #in Meters
follow_distance = 2.0 #in Meters, this is the distance that the drone will remain away from Target while following it 
#Direction relative to the Target 
#Options are NONE, FRONT, FRONT_LEFT, FRONT_RIGHT, BEHIND
direction = Config.FollowDirection.BEHIND
responsiveness =  0.02

missionQueue = Queue(maxsize=50)
droneLocation = [0.0, 0.0, 0.0]


#This list contains fake location coordinates (These coordinates are obtained from mission.py example)
fake_location = [[47.398039859999997,8.5455725400000002],[47.398036222362471,8.5450146439425509],[47.397825620791885,8.5450092830163271]]

async def run():
    
    drone = System()
    await drone.connect(system_address="udp://:14540")

    print("Waiting for drone to connect...")
    async for state in drone.core.connection_state():
        if state.is_connected:
            print(f"Drone discovered with UUID: {state.uuid}")
            break

    print("Waiting for global position estimate...")
    async for health in drone.telemetry.health():
        if health.is_global_position_ok:
            print("Global position estimate ok")
            break
    
    # Start parallel tasks
    print_altitude_task = asyncio.ensure_future(print_altitude(drone))
    print_flight_mode_task = asyncio.ensure_future(print_flight_mode(drone))
    print_health_task = asyncio.ensure_future(print_health(drone))
    #print_coords_task = asyncio.ensure_future(tcp_echo_client(asyncio.get_event_loop()))

    running_tasks = [print_altitude_task, print_flight_mode_task, print_health_task]
    #running_tasks = [print_altitude_task, print_flight_mode_task, print_health_task, print_coords_task]
    termination_task = asyncio.ensure_future(observe_is_in_air(drone, running_tasks))

    
    try:
        print("-- Arming")
        await drone.action.arm()
    except:
        print("-- Already Armed")
    # Execute the maneuvers

    try:
        print("-- Taking off")
        await drone.action.takeoff()
        await asyncio.sleep(10)
    except:
        print("-- Already in air")

    #Follow me Mode requires some configuration to be done before starting the mode
    conf = Config(default_height, follow_distance, direction, responsiveness)
    await drone.follow_me.set_config(conf)

    print("-- Starting Follow Me Mode")

    await drone.follow_me.start()
    await asyncio.sleep(10)
    
    '''
    while missionQueue.qsize() < 2:
        print("Waiting for target coords...")
        await asyncio.sleep(20)


    while missionQueue.qsize() != 0:
        [latitude, longitude] = missionQueue.get()
        print(f"-- {missionQueue.qsize()} target coords left")
        target = TargetLocation(latitude, longitude, 0, 0, 0, 0)
        print (f"-- Following Target: {latitude},{longitude}")
        await drone.follow_me.set_target_location(target)
        await asyncio.sleep(10)
    '''
    
    for latitude,longitude in fake_location:
        target = TargetLocation(latitude, longitude, 0, 0, 0, 0)
        print (f"-- Following Target: {latitude},{longitude}")
        await drone.follow_me.set_target_location(target)
        await asyncio.sleep(2)

    await asyncio.sleep(5)
    print ("-- Stopping Follow Me Mode")
    await drone.follow_me.stop()
    await asyncio.sleep(5)

    print("-- Landing")
    await drone.action.land()

async def print_altitude(drone):
    prev_alt = None
    
    async for position in drone.telemetry.position():
        cur_alt = round(position.relative_altitude_m)
        if cur_alt != prev_alt:
            prev_alt = cur_alt
            print(f"Altitude: {cur_alt}")

        
async def tcp_echo_client(loop):
    reader, writer = await asyncio.open_connection('127.0.0.1', 8888,
                                                   loop=loop)
    while True:
        data = await reader.read(255)
        if data:
            parseData(data.decode())
        await asyncio.sleep(1)
        
def parseData(inputStr):
    print(inputStr)
    if inputStr.find("Coords") != -1:
        data = inputStr
        data = data.replace("'", "")
        data = data.replace(" ", "")
        data = data.replace("RandomCoords:", "")
        coordsStr = data.split(",")
        coords = [float(coordsStr[0]), float(coordsStr[1])]
        print(coords)
        missionQueue.put(coords)


async def print_health(drone):
    battery_percentage = ""
    GPS = ""
    
    async for battery in drone.telemetry.battery():
        battery_percentage = f"Battery: {battery.remaining_percent}"
    async for gps_info in drone.telemetry.gps_info():
        GPS = f"GPS: {gps_info}"
    '''async for position in drone.telemetry.position():
        droneLocation = [position.latitude_deg, position.longitude_deg, position.relative_altitude_m]
        print(droneLocation)'''
    
    print(battery_percentage + GPS)    
    
    await asyncio.sleep(10)
            
async def print_flight_mode(drone):
    previous_flight_mode = None

    async for flight_mode in drone.telemetry.flight_mode():
        if flight_mode is not previous_flight_mode:
            previous_flight_mode = flight_mode
            print(f"Flight mode: {flight_mode}")

async def observe_is_in_air(drone, running_tasks):
    '''Monitors whether the drone is flying or not and
    returns after landing'''

    was_in_air = False

    async for is_in_air in drone.telemetry.in_air():
        if is_in_air:
            was_in_air = is_in_air

        if was_in_air and not is_in_air:
            for task in running_tasks:
                task.cancel()
                try:
                    await task
                except asyncio.CancelledError:
                    pass
            await asyncio.get_event_loop().shutdown_asyncgens()
            return

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(run())