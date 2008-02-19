sseg segment dword stack
	db	100	dup(?)
sseg ends

dseg segment dword
	pstr		db	"Hello World!",'$'
dseg ends

cseg segment dword
	assume cs:cseg,ds:dseg,ss:sseg
start:
	mov	ax,dseg
	mov	ds,ax
	
	mov	dx,offset pstr
	mov	ah,09h
	int	21h

	mov	ah,4Ch
	int	21h
cseg ends
end start