from __future__ import print_function
from dronekit import connect, VehicleMode, LocationGlobalRelative
import time
import socket
#import vehicle.ICD_scripts.findDistance as dist

#Set up option parsing to get connection string
import argparse  
parser = argparse.ArgumentParser(description='Print out vehicle state information. Connects to SITL on local PC by default.')
parser.add_argument('--connect', 
                   help="vehicle connection target string. If not specified, SITL automatically started and used.")
args = parser.parse_args()

connection_string = args.connect
sitl = None




HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

#Start SITL if no connection string specified
if not connection_string:
    import dronekit_sitl as sim
    sitl = sim.start_default()
    connection_string = sitl.connection_string()


# Connect to the Vehicle. 
#   Set `wait_ready=True` to ensure default attributes are populated before `connect()` returns.
print("\nConnecting to vehicle on: %s" % connection_string)
vta = connect(connection_string, wait_ready=True)

vta.wait_ready('autopilot_version')
print("\nConnected to: %s" % connection_string)


def arm(vehicle):
    """
    Arms vehicle.
    """

    print("Basic pre-arm checks")
    # Don't try to arm until autopilot is ready
    while not vehicle.is_armable:
        print(" Waiting for vehicle to initialise...")
        time.sleep(1)

    print("Arming motors")
    # Copter should arm in GUIDED mode
    vehicle.mode = VehicleMode("GUIDED")
    vehicle.armed = True

    # Confirm vehicle armed before attempting to take off
    while not vehicle.armed:
        print(" Waiting for arming...")
        time.sleep(1)
        
def goto(lat, lon, vehicle):
    point = LocationGlobalRelative(lat, lon, 0)
    vta.simple_goto(point)
    print(f"Going to {lat}, {lon}")
    #distance = dist.findDistance(vehicle.location.global_frame.lat, vehicle.location.global_frame.lon, 0, lat, lon, 0)
    #while (distance)

def disarm(vehicle):
    print("Disarming vehicle...")
    vehicle.armed = False


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    running = True
    while running:
        data = s.recv(1024)
        dataStr = data.decode("utf-8")

        print('Received: ', dataStr)
        if "arm" in dataStr and "disarm" not in dataStr:
            arm(vta)
        elif "goto" in dataStr:
            #data: "goto,lat,lon"
            if not vta.armed:
                arm(vta)
            args = dataStr.split(",")
            goto(float(args[1]), float(args[2]), vta)
        elif "quit" in dataStr:
            running = False
            disarm(vta)
        elif "disarm" in dataStr:
            disarm(vta)
            
            
vta.close()
if sitl:
    sitl.stop()