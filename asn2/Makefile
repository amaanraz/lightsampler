all:
	arm-linux-gnueabihf-gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror threads/shutdown.c threads/input.c threads/photores.c modules/leds.c modules/joystick.c modules/misc.c main.c -lpthread -o main -lm
	cp main $(HOME)/cmpt433/public/myApps/asn2

clean:
	rm main