sseg segment dword stack
	db	100	dup(?)
sseg ends

cseg segment dword
	assume cs:cseg,ss:sseg
start:
	mov	ah,02h
	mov	dl,'A'
	int	21h

	mov	ah,4Ch
	int	21h
cseg ends
end start