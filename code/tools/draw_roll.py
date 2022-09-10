#!/usr/bin/env python3

import socket
import sys
from time import time
import matplotlib.pyplot as plt
import numpy as np

if (len(sys.argv) != 2):
    print("Pass duration of the test")
    exit(0)

period = 0.01
duration = float(sys.argv[1])

localIP     = "192.168.0.42"
localPort   = 12345
bufferSize  = 1024

data = []

UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
UDPServerSocket.bind((localIP, localPort))

start = time()
while(time() < (start + duration)):
    bytesAddressPair = UDPServerSocket.recvfrom(bufferSize)
    data.append(float(bytesAddressPair[0].decode())*57.3)

print(max(data))
print(min(data))

# Data for plotting
t = np.arange(0.0, duration, period)

fig, ax = plt.subplots()
if (len(t) <= len(data)):
    ax.plot(t, data[:len(t)])
else:
    ax.plot(t[:len(data)], data)

ax.set(xlabel='time (s)', ylabel='roll angle (rad)',
       title='Θ(t)')
ax.grid()

fig.savefig("test.png")
plt.show()