import numpy as np
import math as math
from geopy import distance
from geographiclib.geodesic import Geodesic ... 

def findDistance(rovLat1,rovLong1,depth1, rovLat2,rovLong2,depth2):
	coord1=(rovLat1,rovLong1) 
	coord2=(rovLat2,rovLong2) 
	d=geopy.distance.vincenty(coord1,coord2).m
	print(d)
	d_2=pow(d,2)
	h=(depth1-depth2)
	h_2=pow(h,2)
	print(math.sqrt(d_2+h_2))
	return math.sqrt(d_2+h_2)
	

def findCOG(rovLat1,rovLong1,rovLat2,rovLong2):
	 COG = Geodesic.WGS84.Inverse(rovLat1, rovLong1, rovLat2,rovLong2)['azi1']  #find the bearing of the path w/ respect to truth north (azimuth)
	 return COG

	 
def findSOG(rovLat1,rovLong1,depth1,rovLat2,rovLong2,depth2,Te): #TE is the time elapsed between the two sample points
	D=findDistance(rovLat1,rovLong1,depth1,rovLat2,rovLong2,depth2) 
	SOG=D/Te
	return SOG #Rate equation Distance/time elapsed S0G

