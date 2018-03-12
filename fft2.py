#!/usr/bin/python
import json
import alsaaudio
import numpy as np
import collections
import sys
import time
import math
import flaschen

rows = 32
cols = 512

ON = (0,255,0)
RED = (255,0,0)
OFF = (0,0,0)

ft = flaschen.Flaschen('10.1.253.237', 1337, 512, 32, transparent=False)

plotMax = 3
offset = 9
plotStart = 0
plotEnd = 512
periodsize = 1500
bufferLength = 4800
sampleRate = 44100
buffer = collections.deque(bufferLength*[0], bufferLength)

inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, alsaaudio.PCM_NORMAL, device='default')
inp.setchannels(1)
inp.setrate(sampleRate)
inp.setformat(alsaaudio.PCM_FORMAT_S16_LE)
periodsize = inp.setperiodsize(periodsize)

for i in range(0, int(bufferLength/periodsize)):
    buffer.extend(np.fromstring(inp.read()[1], dtype=">h"))

with open("./fft_data", "r") as datafile:
    noiseOffset = json.loads(datafile.read())

while True:
    datalength, data = inp.read()
    if (datalength != periodsize):
        continue
    buffer.extend(np.fromstring(data, dtype="<h"))
    fftData = np.absolute(np.fft.rfft(buffer))
    buckets = []
    for i in range(len(fftData)):
        buckets.append(fftData[i]*math.log(i+1))
    buckets = np.array_split(fftData[plotStart:plotEnd], cols)
    buckets = [math.log(sum(x)/(len(x)+1)+1)-offset for x in buckets]
#    with open("./fft_data", "w") as datafile:
#        datafile.write(json.dumps(buckets))
    for i in range(len(buckets)):
        buckets[i] -= noiseOffset[i]
    buckets = [min(int((x*rows)/plotMax),plotMax*rows) for x in buckets]

    for x in range(0,cols,3):
        for y in range(rows):
            if buckets[x] >= y:
                if y >= rows-3:
                    ft.set(x,32-y,RED)
                    ft.set(x+1,32-y,RED)
                else:
                    ft.set(x,32-y,ON)
                    ft.set(x+1,32-y,ON)
            else:
                ft.set(x,32-y,OFF)
                ft.set(x+1,32-y,OFF)

    ft.send()
