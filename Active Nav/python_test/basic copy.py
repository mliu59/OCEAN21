import asyncio
from collections import deque
import math
import sys
import tkinter as tk

def got_stdin_data(a):
    a.append(sys.stdin.readline())

async def main_loop(a):
    
    window = tk.Tk(className='-- Info GUI --')
    window.geometry("500x500")
    
    GUI_task = asyncio.ensure_future(updateGUI(window))
    timer_task = asyncio.ensure_future(timer(window))
    
    while 1:
        data = None
        if len(a) != 0:
            data = a.pop()
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
    command = deque()
    event_loop = asyncio.get_event_loop()
    event_loop.add_reader(sys.stdin, got_stdin_data, command)
    event_loop.run_until_complete(main_loop(command))