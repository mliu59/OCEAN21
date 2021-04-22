import geopy.distance as a
import math

def findDistance(rovLat1,rovLong1,depth1, rovLat2,rovLong2,depth2):
	coord1=(rovLat1,rovLong1) 
	coord2=(rovLat2,rovLong2) 
	d=a.distance(coord1,coord2).km
	#print(d)
	d_2=pow(d,2)
	h=(depth1-depth2)
	h_2=pow(h,2)
	#print(math.sqrt(d_2+h_2))
	return math.sqrt(d_2+h_2)
	
"""
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
	
"""
	
print(findDistance(39.3652081, -76.3390455, 0, 39.365485, -76.3389905, 0))
print(findDistance(39.365485, -76.3389905, 0, 39.3654725, -76.3389301, 0))