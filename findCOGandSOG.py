import numpy as np
import math as math
import geopy 
from geographiclib.geodesic import Geodesic ... 

def findCOG(lat1, lat2, long1, long2):
	 COG = Geodesic.WGS84.Inverse(lat1, long1, lat2, long2)['azi1'] 
	return COG
def SOG(

