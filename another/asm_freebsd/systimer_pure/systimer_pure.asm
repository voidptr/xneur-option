	GLOBAL main

	EXTERN printf

	SECTION .data

	format		db	"Getting system timer: %llu", 10, 0

	SECTION .text

main:
	push	ebp
	push	edi
	push	esi

	mov	ebp, esp

	rdtsc

	push	edx
	push	eax
	push	format
	call	printf
	add	esp, 12

	mov	esp, ebp

	pop	esi
	pop	edi
	pop	ebp
	ret
