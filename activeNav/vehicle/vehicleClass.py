import vehicle.ICD_scripts.findDistance as dist
import vehicle.ICD_scripts.predictTraj as pred
import vehicle.ICD_scripts.convertVels as vels

import time
import copy
import math


def checkArray(oldArr, newArr):
    for i in range(len(oldArr)):
        if newArr[i] != 'NAN':
            oldArr[i] = newArr[i]
    return oldArr

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
        args = dataArgs[2:11]
        print(args)
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
    
def axisDistances(lat1, lon1, dep1, lat2, lon2, dep2):
    #2 is cur, 1 is prev
    ns = dist.findDistance(lat1, lon1, dep1, lat2, lon1, dep1)
    ew = dist.findDistance(lat1, lon1, dep1, lat1, lon2, dep1)
    dep = dist.findDistance(lat1, lon1, dep1, lat1, lon1, dep2)
    
    if (lat1 > lat2):
        ns = -1*ns
    if (lon1 > lon2):
        ew = -1*ew
    if (dep1 > dep2):
        dep = -1*dep
        
    return ns, ew, dep