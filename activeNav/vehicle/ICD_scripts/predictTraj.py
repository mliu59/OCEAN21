import math
import numpy

def predictTraj(x,y,z,vN,vE,vD,t):
	x2=x+vN*t
	y2=y+vE*t
	z2=z+vD*t
	return x2,y2, z2
	
