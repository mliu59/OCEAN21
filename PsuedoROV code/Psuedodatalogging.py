from __future__ import print_function
from dronekit import connect, VehicleMode, LocationGlobalRelative
import time
import csv
import socket

connection_string = '127.0.0.1:14551'

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server


def getROVCoords(vehicle):
    coordsObj = vehicle.location.global_frame
    return [coordsObj.lat, coordsObj.lon, 0]
def getRPY(vehicle):
    rpyObj = vehicle.Attitude
    return [rpyObj.roll, rpyObj.pitch, rpyObj.yaw]
def startConnection(connection_string):
    print("\nConnecting to vehicle on: %s" % connection_string)
    vehicle = connect(connection_string, wait_ready=False, baud=57600, heartbeat_timeout=180)
    vehicle.wait_ready('autopilot_version')
    print("\nConnected to: %s" % connection_string)
    return vehicle


rov = startConnection(connection_string)

with open("ROVdata.csv", "a")as output:
    writer = csv.writer(output, delimiter=",")
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print('Waiting for client connection')
        conn, addr = s.accept()
        with conn:
            while True:
                
                vehiclePos=getROVCoords(rov)
                vehicleRPY=getRPY(rov)
                #vehicleSpeed=rov.velocity
                #vehicleSpeed=[0, 0, 0]
                
                dataString="0,D,"+str(vehiclePos[0])+","+str(vehiclePos[1])+",0.0,0.0,0.0,"+str(vehicleRPY[2])+",0.0,0.0,0.0*"
                
                print(dataString)
            
                csvData=[time.time(),vehiclePos[0],vehiclePos[1],0,0,0,vehicleRPY[2],0,0,0]
                
                
                writer.writerow(csvData)
                #Write your code to send the code to VTA (i think)
                
                conn.sendall(dataString.encode('utf-8'))
                time.sleep(1)