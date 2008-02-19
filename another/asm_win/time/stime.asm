        .686
        .model small
	.code
        
	public _getsystimer
        
; prototype void getsystimer(int *low, int *hight);
_getsystimer proc near
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
_getsystimer endp

end
