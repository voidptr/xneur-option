sseg segment dword stack
	db	100	dup(?)
sseg ends

cseg segment dword
	assume cs:cseg,ss:sseg
start:
	mov	ah,01h
	int	21h
	test	al,al
	jz	short start

	mov	dl,al
	mov	ah,02h
	int	21h

	mov	ah,4Ch
	int	21h
cseg ends
end start