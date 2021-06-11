from collections import deque

command = deque()

command.append("a")
command.append(64)
command.append('lmao')
command.append([123, 342])
print(command)