#!/usr/bin/python
import flaschen
import sounddevice as sd
import numpy
import time
import sys

ROWS = 32
COLS = 512
MAX = 0.25
ON = (255,255,255)
OFF = (1,1,1)

sd.default.samplerate = 44100
sd.default.channels = 1

ft = flaschen.Flaschen('10.1.253.237', 1337, 512, 32, transparent=False)

def display(data):
    pixels = []
    data = [x*ROWS/MAX for x in data]
    for y in range(ROWS//2):
        for x in range(COLS):
            if data[x]+ROWS//2 <= y:
                pixels.append(True)
            else:
                pixels.append(False)

    for y in range(ROWS//2,ROWS):
        for x in range(COLS):
            if y <= data[x]+ROWS//2:
                pixels.append(True)
            else:
                pixels.append(False)
    pixels = [255 if pixels[x//3] else 0 for x in range(len(pixels)*3)]
    ft.set_all(pixels)
    ft.send()

start = time.time()
while True:
    data = sd.rec(COLS, blocking=True)
    tmp = time.time()
    print("{}ms for recording".format(int((tmp-start)*1000)))
    display(data)
    print("{}ms for display".format(int((time.time()-tmp)*1000)))
    now = time.time()
    duration = now - start
    start = now
    print("{} FPS".format(1/duration))
