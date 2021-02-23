import asyncio
from collections import deque
import math
import sys
import tkinter as tk


class info:
    class topic:
        def __init__(self, tag, val):
            self.tag = tag
            self.val = tk.StringVar()
            self.val.set(val)
        def setVal(self, newVal):
            self.val.set(newVal)
            

    def __init__(self, title, items, root):
        self.title = title
        self.list = []
        self.master = root
        for a in items:
            n = self.topic(a[0], a[1])
            self.list.append(n)
    def update(self, tag, newVal):
        for a in self.list:
            if a.tag == tag:
                a.setVal(newVal)
                break
    def display(self):
        tk.Label(self.master, text=self.title).pack()
        for a in self.list:
            tk.Label(self.master, text=a.tag).pack()
            tk.Label(self.master, textvariable=a.val).pack()

class commandDeque:
    def __init__(self):
        self.command = deque()
    def popBottom(self):
        return self.command.pop()
    def popTop(self):
        return self.command.popleft()
    def addTop(self, input):
        self.command.appendleft(input)
    def addBottom(self, input):
        self.command.append(input)
    def count(self):
        return len(self.command)
    def clear(self):
        self.command.clear()
               
def initGUI():
    window = tk.Tk(className='-- Info GUI --')
    window.geometry("500x500")
    
    droneInfo = ['Battery', 'Flight Mode', 'Lat', 'Lon', 'Alt', 'GPS Fix', 'Last Updated']
    ROVInfo = ['Lat', 'Lon', 'Depth']
    a = []
    b = []
    for i in droneInfo:
        a.append([i, 'DEFAULT'])
    for i in ROVInfo:
        b.append([i, 'DEFAULT'])

    dInfo = info('Drone Health + Positional Info', a, window)
    rInfo = info('ROV Info', b, window)
    dInfo.display()
    rInfo.display()
    
    return window
        

async def main_loop():
    
    window = initGUI()
    
    GUI_task = asyncio.ensure_future(updateGUI(window))
    timer_task = asyncio.ensure_future(timer(window))
    
    command = commandDeque()
    
    while 1:
        data = None
        if command.count() != 0:
            data = command.popTop()
        if data == "quit\n":
            print("----- quitting -----")
            break
        print(data)
        await asyncio.sleep(3)

async def timer(root):
    v = tk.StringVar()
    tk.Label(root, textvariable = v).pack()
    counter = 0
    v.set("0")
    while 1:
        counter = counter + 1
        v.set(f"{counter}")
        await asyncio.sleep(1)

async def updateGUI(root, interval=0.5):
    while 1:
        root.update()
        await asyncio.sleep(interval)

if __name__ == "__main__":
    event_loop = asyncio.get_event_loop()
    event_loop.run_until_complete(main_loop())