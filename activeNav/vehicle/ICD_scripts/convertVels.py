import math
import numpy as np

def convertVels(yaw,pitch,roll,vx,vy,vz):
	vW=[]
	VelV=np.array([[vx],[vy],[vz]]) #vehicle velocities
	cp=math.cos(pitch)
	cy=math.cos(yaw)
	sy=math.sin(yaw)
	sp=math.sin(pitch)
	cr=math.cos(roll)
	sr=math.sin(roll)
	R1=np.array([[cp*cy,-1*cp*sy,sp],[cp*sy + cy*sp*sr,cr*cy-sp*sr*sy, -1*cp*sr], [sr*sy-cr*cy*sp,cy*sr+cr*sp*sy,cp*cr]])
	vW=np.matmul(R1,VelV) #find world velocities
	vN=vW[0,0]
	vE=vW[1,0]
	vD=vW[2,0]
	return vN, vE, vD

