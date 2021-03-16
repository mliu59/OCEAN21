import geopy.distance
import math

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
	

	
