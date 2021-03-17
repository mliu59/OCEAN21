import asyncio
from collections import deque
import math
import sys
import tkinter as tk
import time

from mavsdk import System
from mavsdk.follow_me import (Config, FollowMeError, TargetLocation)

testCommands = ["arm",
                "takeoff",
                "hold,5",
                #"goto,47.39803985,8.54557254",
                #"goto,47.39803622,8.54501464",
                #"goto,47.39782562,8.54500928",
                "hold,5",
                #"goto,47.39803985,8.54557254",
                #"goto,47.39803622,8.54501464",
                #"goto,47.39782562,8.54500928",
                "hold,5",
                "land",
                "disarm"]
TEST = True

BASE_ACTION_DELAY = 10
TIME_PER_COORD = 20
GUI_ITEM_WIDTH = 50
COMMAND_DEQUE_MAX_DISPLAY = 10
DATALOG_PERIOD = 5
DATALOG_OUTPUT_MODE = "w" #for overwrite, or "a" for append
DATALOG_FILENAME = "datalog.csv"
COMMANDS = ["arm",   #commands tied to processCommand function
            "takeoff",
            "land",
            "disarm",
            "kill",
            "goto", #followed with args:set target coords
            "hold",
            "startfollow",
            "followtarget",
            "stopfollow"] #followed with args: hold time
DRONE_INFO_TAGS = ["Battery",
                   "Flight_Mode",
                   "Drone_Lat",
                   "Drone_Lon",
                   "Drone_Alt",
                   "GPS_Fix",
                   "Drone_POS_Last_Updated"]
                   #"Origin_Lat",
                   #"Origin_Lon",
                   #"Origin_Alt"]
ROV_INFO_TAGS =   ["ROV_Lat",
                   "ROV_Lon",
                   "ROV_Depth",
                   "ROV_POS_Last_Updated"]
INFO_UPDATE_PERIOD = 1
DEFAULT_HEIGHT = 8.0 #in Meters
FOLLOW_DISTANCE = 2.0 #in Meters, this is the distance that the drone will remain away from Target while following it 
FOLLOW_DIRECTION = Config.FollowDirection.BEHIND #NONE, FRONT, FRONT_LEFT, FRONT_RIGHT, BEHIND
FOLLOW_RESPONSIVENESS = 0.02
DEQUE_RESPOND_PERIOD = 1
HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 8888        # The port used by the server

class info:
    class topic:
        def __init__(self, tag, val):
            self.tag = tk.StringVar(value=tag)
            self.val = tk.StringVar(value=val)
        def setVal(self, newVal):
            self.val.set(newVal)
        def __str__(self):
                return f"{self.tag.get()} :  {self.val.get()}"
            
    def __init__(self, items, root, c):
        self.list = []
        self.master = root
        for a in items:
            n = self.topic(a[0], a[1])
            self.list.append(n)
        self.display(c)        
    def updateVal(self, tag, newVal):
        for a in self.list:
            if a.tag.get() == tag:
                a.setVal(newVal)
                break
    def getVal(self, tag):
        for a in self.list:
            if a.tag.get() == tag:
                return a.val.get()
    def display(self, col):
        for a in self.list:    
            textVarLabel(self.master, a.tag, col, -1)
            textVarLabel(self.master, a.val, col, -1)
    def __str__(self):
        concat = ""
        for a in self.list:
            concat += f"{str(a)}\n"
        return concat
    def getHeaders(self):
        a = ""
        for i in self.list:
            a += f",{i.tag.get()}"
        return a[1:]
    def getVals(self):
        a = ""
        for i in self.list:
            a += f",{i.val.get()}"
        return a[1:]
    
class commandDeque: #addBottom(input), #addTop
    def __init__(self, root, c):
        if TEST:
            self.command = testCommands
        else: 
            self.command = []
        self.meta = [tk.StringVar(), tk.StringVar()]
        self.maxDisplay = COMMAND_DEQUE_MAX_DISPLAY
        self.displayedCommands = []
        self.master = root
        self.current = "None"
        self.display(c)
    def popTop(self):
        a = self.command.pop(0)
        self.current = a
        self.updateDisplay()
        return a
    def addTop(self, input):
        a = [input]
        for element in self.command:
            a.append(element)
        self.command = a
        self.updateDisplay()
    def addBottom(self, input):
        self.command.append(input)
        self.updateDisplay()
    def count(self):
        return len(self.command)
    def clear(self):
        self.command.clear()
        self.updateDisplay()
    def display(self, col):
        textVarLabel(self.master, self.meta[0], col, 0)
        textLabel(self.master, "Current Command:", col, 1)
        textVarLabel(self.master, self.meta[1], col, 2)
        textLabel(self.master, f"Showing Top {self.maxDisplay} Commands:", col, 3)
        for i in range(self.maxDisplay):
            self.displayedCommands.append(tk.StringVar(value=""))
            textVarLabel(self.master, self.displayedCommands[i], col, i + 4)
    def clearCurrent(self):
        self.current = "None"
    def updateDisplay(self):
        self.meta[0].set(f"-- Command Deque (size = {len(self.command)}) --")
        self.meta[1].set(f"{self.current}")
        for i in range(self.maxDisplay):
            if i < len(self.command):
                self.displayedCommands[i].set(self.command[i])
            else:
                self.displayedCommands[i].set("")
    def __str__(self):
        concat = f"CQueue: {self.count()} items\nCurrent: {self.current}"
        for a in self.command:
            concat += f"{str(a)}\n"
        return concat
    def getHeaders(self):
        a = "Current,Deque Size"
        for i in range(self.maxDisplay):
            a += (f",Comnd {i+1}")
        return a
    def getVals(self):
        a = f"{self.current},{str(self.count())}"
        for i in range(self.maxDisplay):
            if i < len(self.command):
                a += f",{self.command[i]}"
            else:
                a += ",None"
        return a
                     
def initGUI():
    window = tk.Tk(className='-- Info GUI --')
    window.geometry("700x600")
    return window
def initInfoPanels(array, title, window, c):
    textLabel(window, title, c, -1)
    a = []
    for i in array:
        a.append([i, 'DEFAULT'])
    Info = info(a, window, c)
    return Info
def textLabel(window, title, c, r):
    if r == -1:
        tk.Label(window, text=title, anchor=tk.W, width=GUI_ITEM_WIDTH).grid(sticky="E", column=c)
    else:
        tk.Label(window, text=title, anchor=tk.W, width=GUI_ITEM_WIDTH).grid(sticky="E", column=c, row=r)
def textVarLabel(window, var, c, r):
    if r == -1:
        tk.Label(window, textvariable=var, anchor=tk.W, width=GUI_ITEM_WIDTH).grid(sticky="E", column=c)
    else:
        tk.Label(window, textvariable=var, anchor=tk.W, width=GUI_ITEM_WIDTH).grid(sticky="E", column=c, row=r)
#def got_stdin_data(q):
#    asyncio.ensure_future(q.put(sys.stdin.readline().replace('\n', '')))

async def processCommand(drone, deque):
    while 1:
        data = None
        if deque.count() != 0:
            command = deque.popTop()
            args = command.split(",")
            if args[0] == COMMANDS[0]:
                try:    
                    await drone.action.arm()
                    await asyncio.sleep(BASE_ACTION_DELAY)
                except: pass
            elif args[0] == COMMANDS[1]:
                try:    
                    await drone.action.takeoff()
                    await asyncio.sleep(BASE_ACTION_DELAY)
                except: pass
            elif args[0] == COMMANDS[2]:
                try:    
                    await drone.action.land()
                    await asyncio.sleep(BASE_ACTION_DELAY)
                except: pass
            elif args[0] == COMMANDS[3]:
                try:    
                    await drone.action.disarm()
                    await asyncio.sleep(BASE_ACTION_DELAY)
                except: pass
            elif args[0] == COMMANDS[4]:
                try:    
                    await drone.action.kill()
                except: pass
            elif args[0] == COMMANDS[5]: #goto
                try:
                    lat = float(args[1])
                    lon = float(args[2])
                    async for terrain_info in drone.telemetry.home():
                        absolute_altitude = terrain_info.absolute_altitude_m
                        break
                    await drone.action.goto_location(lat, lon, absolute_altitude + DEFAULT_HEIGHT, 0)
                    await asyncio.sleep(TIME_PER_COORD)
                except: 
                    print(f"Bad Request: {args}")
                    pass

            elif args[0] == COMMANDS[6]: #hold
                try:    
                    await asyncio.sleep(float(args[1]))
                except:
                    print(f"Bad Request: {args}")
                    pass
            elif args[0] == COMMANDS[7]: #start follow
                try:    
                    conf = Config(DEFAULT_HEIGHT, FOLLOW_DISTANCE, FOLLOW_DIRECTION, FOLLOW_RESPONSIVENESS)
                    await drone.follow_me.set_config(conf)
                    await drone.follow_me.start()
                except: 
                    print(f"Bad Request: {args}")
                    pass
            elif args[0] == COMMANDS[8]: #follow target
                try:
                    target = TargetLocation(float(args[1]), float(args[2]), 0, 0, 0, 0)
                    await drone.follow_me.set_target_location(target)
                except: 
                    print(f"Bad Request: {args}")
                    pass
            elif args[0] == COMMANDS[9]: #stop follow
                try:    
                    await drone.follow_me.stop()
                except: pass
            elif args[0] == "quit":
                print("-- quitting --")
                break
            else:
                print(f"?: {args}")
                await asyncio.sleep(10)
            
        else:
            deque.clearCurrent()
            await asyncio.sleep(DEQUE_RESPOND_PERIOD)
        
        #print(command)
        #await asyncio.sleep(0.1)
        
async def tcp_echo_client(loop):
    reader, writer = await asyncio.open_connection('127.0.0.1', 8888,
                                                   loop=loop)
    while True:
        data = await reader.read(255)
        if data:
            print('Received: %r' % data.decode())

def update_drone_info(drone, info):
    a = asyncio.ensure_future(batteryAsync(drone, info))
    b = asyncio.ensure_future(gpsfixAsync(drone, info))
    c = asyncio.ensure_future(posAsync(drone, info))
    d = asyncio.ensure_future(flightmodeAsync(drone, info))
    #e = asyncio.ensure_future(originAsync(drone, info))
    return [a, b, c, d]
    
async def batteryAsync(drone, info):
    async for battery in drone.telemetry.battery():
        info.updateVal("Battery", battery.remaining_percent)
    await asyncio.sleep(INFO_UPDATE_PERIOD)
async def gpsfixAsync(drone, info):
    async for gps_info in drone.telemetry.gps_info():
        info.updateVal("GPS_Fix", gps_info)
    await asyncio.sleep(INFO_UPDATE_PERIOD) 
async def posAsync(drone, info):    
    async for position in drone.telemetry.position():
        info.updateVal("Drone_Alt", round(position.relative_altitude_m))
        info.updateVal("Drone_Lat", position.latitude_deg)
        info.updateVal("Drone_Lon", position.longitude_deg)
        info.updateVal("Drone_POS_Last_Updated", time.time())
    await asyncio.sleep(INFO_UPDATE_PERIOD)      
async def flightmodeAsync(drone, info):    
    async for flight_mode in drone.telemetry.flight_mode():
        info.updateVal("Flight_Mode", flight_mode)
    await asyncio.sleep(INFO_UPDATE_PERIOD)
"""
async def originAsync(drone, info):    
    async for origin in drone.telemetry.GpsGlobalOrigin():
        info.updateVal("Origin_Lat", origin.latitude_deg)
        info.updateVal("Origin_Lon", origin.longitude_deg)
        info.updateVal("Origin_Alt", origin.altitude_m)
    await asyncio.sleep(INFO_UPDATE_PERIOD)
"""

async def parseCommand(command, q, interval=0.1):
    """
    for parsing command from stdinput before adding it to the command deque
    determines where on the command deque the command will go
    """
    while 1:
        data = await q.get()
        data = data.lower().replace(" ", "")
        args = data.split(",")
        print(args)
        if args[0][0] == '!':
            if args[0][1:] in COMMANDS:
                command.addTop(data[1:])
            else: print("Illegal Command\n")
        else:
            if args[0] in COMMANDS:
                command.addBottom(data)
            else: print("Illegal Command\n")
        await asyncio.sleep(interval)

async def outputToDataLog(infos, command, filename, mod):
    dataLog = open(filename, mod)
    start_time = time.time()
    if mod == "w":
        header = "Time,"
        for a in infos:
            header += (a.getHeaders() + ",")
        header += command.getHeaders()
        dataLog.write(header)
    while 1:
        val = f"\n{int(time.time() - start_time)},"
        for a in infos:
            val += (a.getVals() + ",")
        val += command.getVals()
        dataLog.write(val)
        
        await asyncio.sleep(DATALOG_PERIOD)
            
async def updateGUI(command, root, interval=0.1):
    while 1:
        command.updateDisplay()
        root.update()
        await asyncio.sleep(interval)

async def main_loop(asyncQueue, loop):    
    
    drone = System()
    await drone.connect(system_address="udp://:14550")
    #await drone.connect(system_address = "serial://COM24:57600") #system_address="udp://:14540"
    
    print("Waiting for drone to connect...")
    async for state in drone.core.connection_state():
        if state.is_connected:
            print(f"Drone discovered with UUID: {state.uuid}")
            break

    print("Waiting for global position estimate...")
    async for health in drone.telemetry.health():
        if health.is_global_position_ok:
            print("Global position estimate OK")
            break
    
    print("Initializing GUI & Command Deque...")
    
    window = initGUI()
    dInfo = initInfoPanels(DRONE_INFO_TAGS, '-- Drone Health + Positional Info --', window, 0)
    textLabel(window, "\n---------------\n", 0, -1)
    rInfo = initInfoPanels(ROV_INFO_TAGS, '-- ROV Info --', window, 0)
    command = commandDeque(window, 1)
    
    print("Drone ready for commands")
    
    transfer_task = asyncio.ensure_future(parseCommand(command, asyncQueue))
    GUI_task = asyncio.ensure_future(updateGUI(command, window))
    dataLogging_task = asyncio.ensure_future(outputToDataLog([dInfo, rInfo], command, DATALOG_FILENAME, DATALOG_OUTPUT_MODE))
    updateInfoTasks = update_drone_info(drone, dInfo)
    print_coords_task = asyncio.ensure_future(tcp_echo_client(loop))
    
    mainTask = asyncio.ensure_future(processCommand(drone, command))
    await mainTask

if __name__ == "__main__":
    q = asyncio.Queue()
    event_loop = asyncio.get_event_loop()
    #event_loop.add_reader(sys.stdin, got_stdin_data, q)
    event_loop.run_until_complete(main_loop(q, event_loop))