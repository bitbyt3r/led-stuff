#!/usr/bin/python
import flaschen
import sounddevice as sd
import numpy
import sys

CHUNK = 512
MAX = 0.25
ON = (255,255,255)
OFF = (0,0,0)

sd.default.samplerate = 44100
sd.default.channels = 1

ft = flaschen.Flaschen('10.1.253.237', 1337, 512, 32, transparent=False)

def fft(data):
    output = numpy.absolute(numpy.fft.rfft(list(data)))
    for x in range(512):
        i = output[x]
        height = (i*32/MAX)
        for y in range(32):
            if y < height:
                ft.set(x,y,ON)
            else:
                ft.set(x,y,OFF)
    ft.send()
    print(output[0])

while True:
    fft(sd.rec(512, blocking=True))
