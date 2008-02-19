PROGRAM=mixmatrix

build:
	nasm -f elf ${PROGRAM}.asm
	gcc -Wall -O2 -g0 -o ${PROGRAM} ${PROGRAM}.o
	strip ${PROGRAM}

clean:
	rm ${PROGRAM}.o ${PROGRAM}
