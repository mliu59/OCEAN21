import asyncio
from collections import deque
import math
import sys
import tkinter as tk
import time

from mavsdk import System
from mavsdk.follow_me import (Config, FollowMeError, TargetLocation)

GUI_ITEM_WIDTH = 40
COMMAND_DEQUE_MAX_DISPLAY = 10
DATALOG_PERIOD = 5
DATALOG_OUTPUT_MODE = "w" #for overwrite, or "a" for append
DATALOG_FILENAME = "datalog.csv"
DRONE_INFO_TAGS = ["Battery",
                   "Flight_Mode",
                   "Drone_Lat",
                   "Drone_Lon",
                   "Drone_Alt",
                   "GPS_Fix",
                   "Drone_Last_Updated"]
ROV_INFO_TAGS =   ["ROV_Lat",
                   "ROV_Lon",
                   "ROV_Depth",
                   "ROV_Last_Updated"]
INFO_UPDATE_PERIOD = 1

default_height = 8.0 #in Meters
follow_distance = 2.0 #in Meters, this is the distance that the drone will remain away from Target while following it 
#Direction relative to the Target 
#Options are NONE, FRONT, FRONT_LEFT, FRONT_RIGHT, BEHIND
direction = Config.FollowDirection.BEHIND
responsiveness =  0.02
fake_location = [[47.398039859999997,8.5455725400000002],[47.398036222362471,8.5450146439425509],[47.397825620791885,8.5450092830163271]]


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
    
class commandDeque:
    def __init__(self, root, c):
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
    window.geometry("700x700")
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

def got_stdin_data(q):
    asyncio.ensure_future(q.put(sys.stdin.readline().replace('\n', '')))


async def main_loop(asyncQueue):    
    
    drone = System()
    await drone.connect(system_address="udp://:14540")
    
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

    window = initGUI()
    dInfo = initInfoPanels(DRONE_INFO_TAGS, '-- Drone Health + Positional Info --', window, 0)
    textLabel(window, "\n---------------\n", 0, -1)
    rInfo = initInfoPanels(ROV_INFO_TAGS, '-- ROV Info --', window, 0)
    command = commandDeque(window, 1)
    
    transfer_task = asyncio.ensure_future(parseCommand(command, asyncQueue))
    GUI_task = asyncio.ensure_future(updateGUI(command, window))
    dataLogging_task = asyncio.ensure_future(outputToDataLog([dInfo, rInfo], command, DATALOG_FILENAME, DATALOG_OUTPUT_MODE))
    #print_health_task = asyncio.ensure_future(update_drone_info(drone, dInfo))
    updateInfoTasks = update_drone_info(drone, dInfo)
    #print_coords_task = asyncio.ensure_future(tcp_echo_client(asyncio.get_event_loop()))

    ################


    try:
        print("-- Arming")
        await drone.action.arm()
    except:
        print("-- Already Armed")
    # Execute the maneuvers

    try:
        print("-- Taking off")
        await drone.action.takeoff()
        await asyncio.sleep(10)
    except:
        print("-- Already in air")

    #Follow me Mode requires some configuration to be done before starting the mode
    conf = Config(default_height, follow_distance, direction, responsiveness)
    await drone.follow_me.set_config(conf)

    print("-- Starting Follow Me Mode")

    await drone.follow_me.start()
    await asyncio.sleep(10)
    
    '''
    while missionQueue.qsize() < 2:
        print("Waiting for target coords...")
        await asyncio.sleep(20)


    while missionQueue.qsize() != 0:
        [latitude, longitude] = missionQueue.get()
        print(f"-- {missionQueue.qsize()} target coords left")
        target = TargetLocation(latitude, longitude, 0, 0, 0, 0)
        print (f"-- Following Target: {latitude},{longitude}")
        await drone.follow_me.set_target_location(target)
        await asyncio.sleep(10)
    '''
    
    for latitude,longitude in fake_location:
        target = TargetLocation(latitude, longitude, 0, 0, 0, 0)
        print (f"-- Following Target: {latitude},{longitude}")
        await drone.follow_me.set_target_location(target)
        await asyncio.sleep(2)

    await asyncio.sleep(5)
    print ("-- Stopping Follow Me Mode")
    await drone.follow_me.stop()
    await asyncio.sleep(5)

    print("-- Landing")
    await drone.action.land()
    
    while 1:
        data = None
        if command.count() != 0:
            data = command.popTop()
        else:
            command.clearCurrent()
        if data == "quit":
            print("----- quitting -----")
            break
        asyncio.ensure_future(processCommand(data))
        #print(data)
        await asyncio.sleep(2)

async def processCommand(command):
    ##TODO##
    print(command)
    await asyncio.sleep(0.1)
        
async def tcp_echo_client(loop):
    reader, writer = await asyncio.open_connection('127.0.0.1', 8888,
                                                   loop=loop)
    while True:
        data = await reader.read(255)
        if data:
            parseData(data.decode())
        await asyncio.sleep(1)
        
def parseData(inputStr):
    ##TODO##
    return

def update_drone_info(drone, info):
    a = asyncio.ensure_future(batteryAsync(drone, info))
    b = asyncio.ensure_future(gpsfixAsync(drone, info))
    c = asyncio.ensure_future(posAsync(drone, info))
    d = asyncio.ensure_future(flightmodeAsync(drone, info))
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
    await asyncio.sleep(INFO_UPDATE_PERIOD)   
    
async def flightmodeAsync(drone, info):    
    async for flight_mode in drone.telemetry.flight_mode():
        info.updateVal("Flight_Mode", flight_mode)
    await asyncio.sleep(INFO_UPDATE_PERIOD)

async def parseCommand(command, q, interval=0.1):
    while 1:
        data = await q.get()
        
        ##############TODO###################
        ## data processing ##
        
        command.addBottom(data)
        
        ######################
        
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

if __name__ == "__main__":
    q = asyncio.Queue()
    event_loop = asyncio.get_event_loop()
    event_loop.add_reader(sys.stdin, got_stdin_data, q)
    event_loop.run_until_complete(main_loop(q))