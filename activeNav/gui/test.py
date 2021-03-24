import tkinter as tk
import time

window = tk.Tk(className='-- Info GUI --')
window.geometry("700x600")
var = tk.StringVar(value="0")
tk.Label(window, textvariable=var, anchor=tk.W).pack()
window.update()
#window.mainloop()

time.sleep(5)

for i in range(10):
    var.set(str(i))
    window.update()
    time.sleep(2)
    
    
    
time.sleep(5)