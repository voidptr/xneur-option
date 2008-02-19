PROGRAM=systimer_pure

build:
	nasm -f elf ${PROGRAM}.asm
	gcc -Wall -O2 -g0 -o ${PROGRAM} ${PROGRAM}.o

clean:
	rm ${PROGRAM}.o ${PROGRAM}
