from __future__ import print_function
from dronekit import connect, VehicleMode, LocationGlobalRelative
import time
import csv
import socket
import activeNav.vehicle.vehicleClass as vehicleClass

connection_string = '127.0.0.1:14551'

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

class vehicle:
    
    coords = [0.0, 0.0, 0.0] #lat, lon, depth
    lastUpdated = 0
    
    def __init__(self, lat=0.0, lon=0.0, depth=0.0):
        self.coords = checkArray(self.coords, [lat, lon, depth])
    
    def getCoords(self):
        return self.coords
    
    def updateCoords(self, newCoords, maxRefresh):
        if (time.time() - self.lastUpdated > maxRefresh):
            self.setCoords(newCoords)
 
    def setCoords(self, newCoords):
        self.coords = checkArray(self.coords, newCoords)
        self.updateTime()
    
    def getDistance(self, a):
        return dist.findDistance(self.coords[0], self.coords[1], self.coords[2], a.coords[0], a.coords[1], a.coords[2])
    
    def updateTime(self):
        self.lastUpdated = time.time()
    

            
        
class rov(vehicle):
    
    velocity = [0.0, 0.0, 0.0]
    rpy = [0.0, 0.0, 0.0]
    
    def __init__(self, lat=0.0, lon=0.0, depth=0.0):
        super().__init__(lat, lon, depth)
        
    def updateROV(self, dataArgs):
        # args is an array with all fields
        #[lat, lon, depth, roll, pitch, yaw, xv, yv, zv]
        args = dataArgs[2:10]
        self.setCoords([args[0], args[1], args[2]])
        self.velocity = checkArray(self.velocity, [args[3], args[4], args[5]])
        self.rpy = checkArray(self.rpy, [args[6], args[7], args[8]])
        self.updateTime()

    def copyROV(self, a):
        self.coords = copy.deepcopy(a.coords)
        self.velocity = copy.deepcopy(a.velocity)
        self.rpy = copy.deepcopy(a.rpy)
        self.lastUpdated = a.lastUpdated
        
    def watchdogFlag(self, lastROV, watchdogThreshold):
        vN, vE, vD = vels.convertVels(lastROV.rpy[0], lastROV.rpy[1], lastROV.rpy[2], lastROV.velocity[0], lastROV.velocity[1], lastROV.velocity[2])
        relN, relW, relDep = axisDistances(lastROV.coords[0], lastROV.coords[1], lastROV.coords[2], self.coords[0], self.coords[1], self.coords[2])
        predN, predW, predDep = pred.predictTraj(0, 0, 0, vN, vE, vD, self.lastUpdated - lastROV.lastUpdated)
        
        distDiff = math.sqrt((relN - predN)**2 + (relW - predW)**2 + (relDep - predDep)**2)
        
        if (distDiff > watchdogThreshold):
            return True
        
        return False

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

def getROVCoords(vehicle):
    coordsObj = vehicle.location.global_frame
    return [coordsObj.lat, coordsObj.lon, 0]


   
while True:
	vehiclePos=getROVCoords(ROV)
	vehicleRPY=rov.rpy
	vehicleSpeed=rov.velocity
	dataString=str(vehiclePos[0])+","+str(vehiclePos[1])+","+str(0)+","+str(0)+","+str(0)+","+str(vehicleRPY[2])+","+str(vehicleSpeed[0])+","+str(vehicleSpeed[1])+","+str(0)+","+str("*")
	#print()
	csvData=[vehiclePos[0],vehiclePos[1],0,0,0,vehicleRPY[2],vehicleSpeed[0],vehicleSpeed[1],0]
	with open("ROVdata.csv", "a")as output:
		writer = csv.writer(output, delimiter=",")
		writer.writerow(csvData)
	#Write your code to send the code to VTA (i think)
	time.sleep(1000)
