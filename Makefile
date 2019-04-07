
all:
	gcc -Wall -o radio radio.c -lwiringPi

clean:
	rm -f radio
