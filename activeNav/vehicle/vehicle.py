import ICD_scripts as calc
import time


def checkArray(oldArr, newArr):
    for i in range(len(oldArr)):
        if newArr[i] != 'NAN':
            oldArr[i] = newArr[i]
    return oldArr

class vehicle:
    
    coords = [0.0, 0.0, 0.0] #lat, lon, depth
    
    def __init__(self, lat=0.0, lon=0.0, depth=0.0):
        self.coords = checkArray(self.coords, [lat, lon, depth])
    
    def getCoords(self):
        return self.coords
    
    def setCoords(self, newCoords):
        self.coords = checkArray(self.coords, newCoords)
        
    def getDistance(self, a):
        return calc.findDistance.findDistance(self.coords[0], self.coords[1], self.coords[2], a.coords[0], a.coords[1], a.coords[2])
        
class rov(vehicle):
    
    velocity = [0.0, 0.0, 0.0]
    rpy = [0.0, 0.0, 0.0]
    lastUpdated = 0
    
    def __init__(self, lat=0.0, lon=0.0, depth=0.0):
        super().__init__(lat, lon, depth)
        
    def update(self, args):
        # args is an array with all fields
        #[lat, lon, depth, roll, pitch, yaw, xv, yv, zv]
        self.setCoords([args[0], args[1], args[2]])
        self.velocity = checkArray(self.velocity, [args[3], args[4], args[5]])
        self.rpy = checkArray(self.rpy, [args[6], args[7], args[8]])
        self.lastUpdated = time.time()