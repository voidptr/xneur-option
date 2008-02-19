sseg	segment	dword	stack
	db	100	dup(?)
sseg ends

dseg	segment	dword
	estr		db	"Please enter string: $"
	istr		db	0Dh,0Ah,"You input: $"

	buffer_len	db	253
	input_len	db	?
	buffer		db	252	dup(?)
dseg ends

cseg	segment	dword
	assume cs:cseg,ds:dseg,ss:sseg
start:
	mov	ax,dseg
	mov	ds,ax

	mov	dx,offset estr
	mov	ah,09h
	int	21h
	
	mov	ah,0Ah
	mov	dx,offset buffer_len
	int	21h

	mov	cl,input_len			; CL = LEN - правая граница обхода
	dec	cl
	jle	short exend			; Если CL<2, то ничего не делаем
	xor	bx,bx				; BX = 0 - левая граница обхода

exchg:
	mov	dl,ds:buffer[bx]		; DL = buffer[i]
	push	bx
	mov	bl,cl
	mov	dh,ds:buffer[bx]		; DH = buffer[len-i]
	mov	ds:buffer[bx],dl		; buffer[len-i]=DL
	pop	bx
	mov	ds:buffer[bx],dh		; buffer[i]=DH
	inc	bl
	dec	cl
	cmp	cl,bl
	jg	short exchg

exend:
	mov	dx,offset istr
	mov	ah,09h
	int	21h

	mov	dx,offset buffer
	mov	cl,input_len
	mov	bx,1
	mov	ah,40h
	int	21h

	mov	ah,4Ch
	int	21h
cseg	ends
end	start