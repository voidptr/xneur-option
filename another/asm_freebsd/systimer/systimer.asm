	GLOBAL	getsystimer

getsystimer:
	push	ebp
	mov	ebp, esp

	rdtsc

	mov	ebx, [ebp + 8]
	mov	[ebx], eax

	mov	ebx, [ebp + 12]
	mov	[ebx], edx

	mov	esp, ebp
	pop	ebp
	ret
