sseg segment dword stack
	db	100	dup(?)
sseg ends

dseg	segment	dword
	startx		dw	0
	starty		dw	0
	endx		dw	319
	endy           	dw	199
;	endx		dw	100
;	endy           	dw	200
	lwidth		dw	?
	lheight		dw	?
dseg ends

cseg segment dword
	assume	cs:cseg,ds:dseg,ss:sseg

PutPixel	proc	near
       	mov	ah,0Ch
	mov	al,02h
	mov	dx,starty
	mov	cx,startx
	int	10h

	ret
PutPixel	endp

start:
	push	dseg
	pop	ds

	mov	ax,startx
	mov	bx,endx
	cmp	bx,ax
	jg	widthok			; ���� startX ������ endX
	
	mov     startx,bx		; ������ �������
	mov	endx,ax
widthok:

	mov	ax,starty
	mov	bx,endy
	cmp	bx,ax
	jg	heightok		; ���� startY ������ endY

	mov     starty,bx		; ������ �������
	mov	endy,ax
heightok:

	mov	ax,0013h
	int	10h

	call	PutPixel

	mov	ax,endx
	sub	ax,startx		; AX - ������
	mov	lwidth,ax
	
	mov	cx,endy
	sub	cx,starty		; CX - ������
	mov	lheight,cx

	cmp	ax,cx			; ��� ������ - ������ ��� ������?
	jg	short widthgreater	; ������...

	mov	bx,lheight		; ������...

	test	bx,bx			; ������ �����?
	jz	short enddraw

	mov	ax,lwidth
	add	ax,lwidth
	sub	ax,lheight

drawlineh:

	inc	starty

	cmp	ax,0
	jl	short firsth

	inc	startx

	sub	ax,lheight
	sub	ax,lheight

firsth:
	add	ax,lwidth
	add	ax,lwidth

	push	ax
	call	PutPixel
	pop	ax

	dec	bx
	jnz	drawlineh
	jmp	enddraw
	
widthgreater:

	mov	bx,lwidth

	test	bx,bx			; ������ �����?
	jz	short enddraw
	
	mov	ax,lheight
	add	ax,lheight
	sub	ax,lwidth

drawlinew:

	inc	startx

	cmp	ax,0
	jl	short firstw
	
	inc	starty

	sub	ax,lwidth
	sub	ax,lwidth
firstw:

	add	ax,lheight
	add	ax,lheight

	push	ax
	call	PutPixel
	pop	ax

	dec	bx
	jnz	drawlinew

enddraw:
	mov	ah,01h
	int	21h
	
	mov	ax,0003h
	int	10h

exit:
	mov	ah,4Ch
	int	21h
cseg ends
end start