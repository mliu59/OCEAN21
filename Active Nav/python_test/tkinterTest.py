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

#print(a)
#print(b)

dInfo = info('Drone Health + Positional Info', a, window)
rInfo = info('ROV Info', b, window)
dInfo.display()
rInfo.display()

window.mainloop()