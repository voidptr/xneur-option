sseg segment dword stack
	db	100	dup(?)
sseg ends

dseg segment dword
	estr		db	"Please enter string: $"
	istr		db	0Dh,0Ah,"You input: $"
	buffer_len	db	254
	input_len	db	?
	buffer		db	252	dup(?)
dseg ends

cseg segment dword
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
	
	mov	dx,offset istr
	mov	ah,09h
	int	21h

	mov	dx,offset buffer
	mov	cl,input_len
	mov	ah,40h
	int	21h

	mov	ah,4Ch
	int	21h
cseg ends
end start