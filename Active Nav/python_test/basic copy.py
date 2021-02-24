import asyncio
from collections import deque
import math
import sys
import tkinter as tk
import time

GUI_ITEM_WIDTH = 40
DATA_LOG_PERIOD = 5
COMMAND_DEQUE_MAX_DISPLAY = 10
DATALOG_OUTPUT_MODE = "w" #for overwrite, or "a" for append

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
    window = initGUI()
    droneInfo = ['Battery', 'Flight Mode', 'Lat', 'Lon', 'Alt', 'GPS Fix', 'Last Updated']
    ROVInfo = ['Lat', 'Lon', 'Depth', 'Last Updated']
    dInfo = initInfoPanels(droneInfo, '-- Drone Health + Positional Info --', window, 0)
    textLabel(window, "\n---------------\n", 0, -1)
    rInfo = initInfoPanels(ROVInfo, '-- ROV Info --', window, 0)
    command = commandDeque(window, 1)
    
    transfer_task = asyncio.ensure_future(processCommand(command, asyncQueue))
    GUI_task = asyncio.ensure_future(updateGUI(command, window))
    dataLogging_task = asyncio.ensure_future(outputToDataLog([dInfo, rInfo], command, "datalog.csv", DATALOG_OUTPUT_MODE))
    
    while 1:
        data = None
        if command.count() != 0:
            data = command.popTop()
        else:
            command.clearCurrent()
        if data == "quit":
            print("----- quitting -----")
            break
        #print(data)
        await asyncio.sleep(2)
    

async def processCommand(command, q, interval=0.1):
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
        val = f"\n{time.time() - start_time},"
        for a in infos:
            val += (a.getVals() + ",")
        val += command.getVals()
        dataLog.write(val)
        
        await asyncio.sleep(DATA_LOG_PERIOD)
            

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