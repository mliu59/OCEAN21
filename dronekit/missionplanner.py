#print("Start simulator (SITL)")
#import dronekit_sitl
import time
#sitl = dronekit_sitl.start_default()
#connection_string = sitl.connection_string()
connection_string = '127.0.0.1:14551'

# Import DroneKit-Python
from dronekit import connect, VehicleMode

# Connect to the Vehicle.
print("Connecting to vehicle on: %s" % (connection_string,))
vehicle = connect(connection_string, wait_ready=True)
#vehicle = connect(connection_string, wait_ready=True, baud=9600)

while True:
    time.sleep(2)
    print(time.time())
    print(" GPS: %s" % vehicle.gps_0)
    print(" Battery: %s" % vehicle.battery)
    print(" Last Heartbeat: %s" % vehicle.last_heartbeat)
    print(" Is Armable?: %s" % vehicle.is_armable)
    print(" System status: %s" % vehicle.system_status.state)
    print(" Mode: %s" % vehicle.mode.name)