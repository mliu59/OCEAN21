from __future__ import print_function
from dronekit import connect, VehicleMode, LocationGlobalRelative
import time
import socket
import vehicle.ICD_scripts.findDistance as dist
import vehicle.ICD_scripts.predictTraj as pred

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
print("\nConnected to: 127.0.0.1:14551")
#print("\nConnecting to vehicle on: %s" % connection_string)
#vta = connect(connection_string, wait_ready=True)
vta = connect('127.0.0.1:14551', wait_ready=False, baud=57600, heartbeat_timeout=180)

vta.wait_ready('autopilot_version')
#print("\nConnected to: %s" % connection_string)

#variables
#float forceThresh=
#float tetherLength=


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
def getForce():
    #
    return forceReading

def triggerTugSteering(vehicle):


def activeNavigation(vehicle, vehicle): #input should be both VTA and ROV
   # rovDistance=dist.findDistance(vl_frame.long,vehicle.location.global_frame.depth, ROV)  irst and then going to fix the code
   storeROVstate=rov.posdata
   while (rovDistance>thresholdDistance): 
        #write function to get data state
        forceReading=getForce()
        if forceReading>forceThresh
            triggerTugSteering()
            break 

         #if data sucks, then break while loop, or force reading beyond threshold switch to tug steering
         #checkDataQual check data quality 
         predDist=pred.predictTraj(rov.lat,rov.long,rov.depth,rov.vmax,rov.vmax,rov.max,1) 
         
         time.sleep(1000)
         dist=dist.findDistance(rov.lat,rov.long,rov.depth,rovX,rovY,rovZ)
         if dist>predDist#define
             triggerTugSteering()
         vta.goto(rov.lat,rov.long,vta)
         pos=vta.UpdatePOS
         pos=ROV.updatePOS
         while (vta Update POS returns null or rov pos update returns null) #hard_code Heartbeat timeout
            int i=0
            vta.UpdatePOS
            rov.UpdatePOS
            i=+
            time.sleep(1000)
            if i>10
                triggerTugSteering()
         rovDistance=dist.findDistance(vehicle.localation.global_frame.lat,vehicle.localation.global_frame.long,vehicle.location.global_frame.depth, ROV)  ##I don't know if I'm calling this right, trying to write the algo first and then going to fix the code

arm(vta)
time.sleep(20)
goto(39.3628608, -76.339424, vta)
time.sleep(60)


"""
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
 """     


            
vta.close()
if sitl:
    sitl.stop()