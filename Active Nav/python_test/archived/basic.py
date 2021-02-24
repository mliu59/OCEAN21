import asyncio
from mavsdk import System
from mavsdk.follow_me import (Config, FollowMeError, TargetLocation)
import math
import sys

default_height = 2.0 #in Meters
follow_distance = 10.0 #in Meters, this is the distance that the drone will remain away from Target while following it 
#Direction relative to the Target 
#Options are NONE, FRONT, FRONT_LEFT, FRONT_RIGHT, BEHIND
direction = Config.FollowDirection.BEHIND
responsiveness =  0.02

#This list contains fake location coordinates (These coordinates are obtained from mission.py example)
fake_location = [[47.398039859999997,8.5455725400000002],[47.398036222362471,8.5450146439425509],[47.397825620791885,8.5450092830163271]]

def got_stdin_data(q):
    asyncio.ensure_future(q.put(sys.stdin.readline()))

async def main_loop():
    
    drone = System()
    await connect_drone(drone)
    

    
    
    print("Initializing tasks")
    # Start parallel tasks
    print_altitude_task = asyncio.ensure_future(print_altitude(drone))
    print_flight_mode_task = asyncio.ensure_future(print_flight_mode(drone))
    print_battery_task = asyncio.ensure_future(print_battery(drone))
    print_GPSfix_task = asyncio.ensure_future(print_gpsfix(drone))
    
    running_tasks = [print_altitude_task, print_flight_mode_task, print_battery_task, print_GPSfix_task]
    #running_tasks = [print_altitude_task, print_flight_mode_task, print_health_task, print_coords_task]
    termination_task = asyncio.ensure_future(observe_is_in_air(drone, running_tasks))

    # Execute the maneuvers
    print("-- Arming")
    await drone.action.arm()

    #Follow me Mode requires some configuration to be done before starting the mode
    conf = Config(default_height, follow_distance, direction, responsiveness)
    await drone.follow_me.set_config(conf)

    print("-- Taking off")
    await drone.action.takeoff()

    await asyncio.sleep(10)
    
    print("-- Starting Follow Me Mode")

    await drone.follow_me.start()
    await asyncio.sleep(10)

    for latitude,longitude in fake_location:
        target = TargetLocation(latitude, longitude, 0, 0, 0, 0)
        print (f"-- Following Target: {latitude},{longitude}")
        await drone.follow_me.set_target_location(target)
        await asyncio.sleep(2)

    await asyncio.sleep(5)
    print ("-- Stopping Follow Me Mode")
    await drone.follow_me.stop()
    await asyncio.sleep(100)
    
    await drone.action.home()

    print("-- Landing")
    await drone.action.land()

async def connect_drone(drone):
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

async def print_altitude(drone):
    prev_alt = None
    
    async for position in drone.telemetry.position():
        cur_alt = round(position.relative_altitude_m)
        if cur_alt != prev_alt:
            prev_alt = cur_alt
            print(f"Altitude: {cur_alt}")

async def print_battery(drone):
    async for battery in drone.telemetry.battery():
        await asyncio.sleep(5)
        print(f"Battery: {battery.remaining_percent}")
        
async def print_gpsfix(drone):
    async for gps_info in drone.telemetry.gps_info():
        await asyncio.sleep(5)
        print(f"{gps_info}")
            
async def print_flight_mode(drone):
    previous_flight_mode = None

    async for flight_mode in drone.telemetry.flight_mode():
        if flight_mode is not previous_flight_mode:
            previous_flight_mode = flight_mode
            print(f"Flight mode: {flight_mode}")


async def observe_is_in_air(drone, running_tasks):
    """ Monitors whether the drone is flying or not and
    returns after landing """

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
 
    q = asyncio.Queue()
    event_loop = asyncio.get_event_loop()
    event_loop.add_reader(sys.stdin, got_stdin_data, q)
    
    try:
        event_loop.run_until_complete(main_loop())
    finally:
        event_loop.close()