all: wave

wave: wave.c
	gcc wave.c -lportaudio -o wave
