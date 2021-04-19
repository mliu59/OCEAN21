from __future__ import print_function
from dronekit import connect, VehicleMode, LocationGlobalRelative
import time
import socket
#import activeNav.vehicle.ICD_scripts.findDistance as dist
#import activeNav.vehicle.ICD_scripts.predictTraj as pred
import vehicle.vehicleClass as vehicleClass

connection_string = '127.0.0.1:14551'

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

tetherLength = 30  # The tether length in meters
heartbeatTimeout = 10 # amount of time to wait for heartbeat before activating tug steering
activeNavRestartTimer = 600 # amount of time to wait before restarting active Nav
expectedROVRefresh = 1.0
vtaMaxRefresh = expectedROVRefresh / 2
watchdogThreshold = 50 # in meters

vtaThreshold = tetherLength / 2 # in meters


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
    vehicle.simple_goto(point)
    print(f"Going to {lat}, {lon}")
    #distance = dist.findDistance(vehicle.location.global_frame.lat, vehicle.location.global_frame.lon, 0, lat, lon, 0)
    #while (distance)

def disarm(vehicle):
    print("Disarming vehicle...")
    vehicle.armed = False
    
def checkForce():
    #float forceThresh=
    # TODO
    return True
def triggerTugSteering():
    return

def dataValidator(dataStr):
    # args is an array with all fields
    #[lat, lon, depth, roll, pitch, yaw, xv, yv, zv]
    dataQual = True
    dataQuit = False
    
    dataArgs = dataStr.split(',')
    if len(dataArgs) != 11:
        return False, False, dataArgs
    
    if (dataArgs[10][-1] != "*"):
        #print("string no end char")
        dataQual = False
    else:
        dataArgs[10] = dataArgs[10][0:-1]
    
    if (dataArgs[0] != '0' or (dataArgs[1] != 'D' and dataArgs[1] != 'K')):
        dataQual = False
        #print("first fields error")
    
        
    if (dataArgs[1] == "K"):
        dataQuit = True
        #print("quitting")
        
    for i in range(2, 11):
        try:
            if (dataArgs[i] != "NAN"):
                dataArgs[i] = float(dataArgs[i])
        except:
            #print("cannot cast field %f" % i)
            dataQual = False
            break

    return dataQuit, dataQual, dataArgs

def getVTACoords(vehicle):
    coordsObj = vehicle.location.global_frame
    return [coordsObj.lat, coordsObj.lon, 0]
    
def readData(s):
    data = s.recv(1024)
    dataStr = data.decode("utf-8")
    print('Received: ', dataStr)
    return dataStr

def startConnection(connection_string):
    print("\nConnecting to vehicle on: %s" % connection_string)
    vehicle = connect(connection_string, wait_ready=False, baud=57600, heartbeat_timeout=180)
    vehicle.wait_ready('autopilot_version')
    print("\nConnected to: %s" % connection_string)
    return vehicle
    

def mainLoop():
    
    vtaConnection = startConnection(connection_string)
    arm(vtaConnection)
    
    vta = vehicleClass.vehicle()
    lastROV = vehicleClass.rov()
    tempROV = vehicleClass.rov()
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        print("\nConnected to ROV server: " + HOST + " , " + str(PORT))
        
        running = True
        
        while running:
            
            while True:
                
                #variable for checking if in this loop, rov data was updated
                newData = False
                dataStr = readData(s)
                
                
                #check the qualities of the data string
                QUIT, dataQual, dataArgs = dataValidator(dataStr)
                #print(dataArgs)
                #print(QUIT, dataQual)
                
                #if data is kill message, stop active Nav entirely                
                if QUIT:
                    running = False
                    break
                
                #if data quality is good
                if dataQual:
                    #update the temp ROV object with new data
                    tempROV.updateROV(dataArgs)
                    
                    #check watchdog if the temp ROV is a valid data point
                    #if not tempROV.watchdogFlag(lastROV, watchdogThreshold):
                    lastROV.copyROV(tempROV)
                    newData = True
                    #print(lastROV.getCoords())
                    #print(lastROV.velocity)
                    #print(lastROV.rpy)
                    #if not, ignore data and do nothing
                
                #update the VTA's position
                vta.updateCoords(getVTACoords(vtaConnection), vtaMaxRefresh)
                
                
                if newData:
                    if vtaThreshold < vta.getDistance(lastROV):
                        goto(lastROV.coords[0],lastROV.coords[1],vtaConnection)
                
                #if the ROV position was updated this loop
                #check if the distance between the vta and the rov is greater than the tetherLength
                #if so, stop this iteration of activeNav
                #if newData and tetherLength < vta.getDistance(lastROV):
                #    running = False
                #    break
                
                #if timeout, stop this iteration of activeNav
                if time.time() - lastROV.lastUpdated > heartbeatTimeout:
                    break
                
                
                    
                    
                
                
            time.sleep(activeNavRestartTimer)
            
            
            
            
            
    
    print("Closing script")
    vtaConnection.close()
    
    #time.sleep(20)
    #goto(39.3628608, -76.339424, vtaConnection)
    #time.sleep(60)
"""
def activeNavigation(vta, rov): #input should be both VTA and ROV
    # rovDistance=dist.findDistance(vl_frame.long,vehicle.location.global_frame.depth, ROV)  irst and then going to fix the code
    storeROVstate=rov.posdata
    while (rovDistance>thresholdDistance): 
        #write function to get data state
        if not checkForce():
            triggerTugSteering()
            break 

        #if data sucks, then break while loop, or force reading beyond threshold switch to tug steering
        #checkDataQual check data quality 
        predDist=pred.predictTraj(rov.lat,rov.long,rov.depth,rov.vmax,rov.vmax,rov.max,1) 
         
        time.sleep(1000)
        dist=dist.findDistance(rov.lat,rov.long,rov.depth,rovX,rovY,rovZ)
        if dist>predDist: #define
            triggerTugSteering()
        vta.goto(rov.lat,rov.long,vta)
        pos=vta.UpdatePOS
        pos=ROV.updatePOS
        while (vta Update POS returns null or rov pos update returns null) : #hard_code Heartbeat timeout
            i = 0
            vta.UpdatePOS
            rov.UpdatePOS
            i = i + 1
            time.sleep(1000)
            if i>10 :
                triggerTugSteering()
        rovDistance=dist.findDistance(vehicle.localation.global_frame.lat,vehicle.localation.global_frame.long,vehicle.location.global_frame.depth, ROV)  ##I don't know if I'm calling this right, trying to write the algo first and then going to fix the code

    return

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

if __name__ == "__main__":
    mainLoop()