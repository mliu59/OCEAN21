from __future__ import print_function
from dronekit import connect, VehicleMode, LocationGlobalRelative
import time

#Set up option parsing to get connection string
import argparse  
parser = argparse.ArgumentParser(description='Print out vehicle state information. Connects to SITL on local PC by default.')
parser.add_argument('--connect', 
                   help="vehicle connection target string. If not specified, SITL automatically started and used.")
args = parser.parse_args()

connection_string = args.connect
sitl = None


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
        


arm(vta)

print("Set default/target airspeed to 3")
vta.airspeed = 3

print("Going towards first point for 30 seconds ...")
point1 = LocationGlobalRelative(-35.361354, 149.165218, 20)
vta.simple_goto(point1)

# sleep so we can see the change in map
time.sleep(30)

print("Going towards second point for 30 seconds (groundspeed set to 10 m/s) ...")
point2 = LocationGlobalRelative(-35.363244, 149.168801, 20)
#vta.simple_goto(point2, groundspeed=10)
vta.simple_goto(point2)

# sleep so we can see the change in map
time.sleep(30)

#print("Returning to Launch")
#vta.mode = VehicleMode("RTL")

# Close vehicle object before exiting script
print("Close vehicle object")
vta.close()

# Shut down simulator if it was started.
if sitl:
    sitl.stop()
    
#vehicle.reboot()


