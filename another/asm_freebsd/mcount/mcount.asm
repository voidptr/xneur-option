	GLOBAL main

	EXTERN printf

	SECTION .data

	msize		dd	16

	mass		dd	5.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 16.0

	format		db	"Elements count: %d", 10, 0

	SECTION .text

main:
	push	ebp
	push	edi
	push	esi

	mov	ebp, esp

	xor	ebx, ebx
	xor	edx, edx		; Счётчик элементов

	finit
	fld	dword [mass]		; Помещаем 0-й элемент в стек FPU
next_element:
	fcom	dword [mass + edx * 4]
	fstsw	ax
	sahf
	jb	short notinc				; Если mass[0] < mass[i]

	inc	ebx
notinc:

	inc	edx
	cmp	edx, dword [msize]
	jl	short next_element
	
	push	ebx
	push	format
	call	printf
	add	esp, 8

	mov	esp, ebp

	pop	esi
	pop	edi
	pop	ebp
	ret
