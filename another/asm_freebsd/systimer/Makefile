build:
	nasm -f elf systimer.asm
	gcc -Wall -O2 -std=c99 -g0 -o systimer systimer.c systimer.o
	strip systimer

clean:
	rm systimer.o systimer
