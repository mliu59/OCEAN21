use MAVProxy to reroute COM10 port connection to UDP

windows command prompt:
	mavproxy --master=COM10 --out 127.0.0.1:14550 --out 127.0.0.1:14551
COM10 is the connection port of the PixHawk or Radio, check in windows device manager
reroute the signal to UDP localhost:14550 & 14551
14550 goes to mission control
14551 goes to control software
Having the same connection freezes Mission Control, both ports necessary
https://dronekit.netlify.app/develop/sitl_setup.html


VTA information will be displayed on Mission Planner (Status tab)
