sseg	segment	dword	stack
	db	100	dup(?)
sseg ends

dseg	segment	dword
	efname		db	"Please enter filename: $"
	erropfmsg	db	"Error opening file$"
	errreadfmsg	db	"Error reading file$"
	rn		db	0Dh,0Ah,'$'

	buffer_len	db	253
	input_len	db	?
	buffer		db	253	dup(?)
dseg ends

cseg	segment	dword
	assume cs:cseg,ds:dseg,ss:sseg
start:
	mov	ax,dseg
	mov	ds,ax

again:
	mov	dx,offset efname
	mov	ah,09h
	int	21h				; Please enter filename
	
	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; ���������� �����
	
 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; ������� \r\n
	
	xor     bx,bx
	mov	bl,input_len
	test	bl,bl
	jz	short again			; ���� �� ����� ������, �� �������
	
	mov	ds:buffer[bx],0			; ��������� ����� �����

	mov	al,10000100b			; ����� �������
	mov	dx,offset buffer
	mov	ah,3Dh
	int	21h				; �������� �����
	jc	short erropf			; ���� ������� �� �������, �� ������

	mov	dx,offset buffer		; � DX - �������� ������ � DS
	push	ax

readloop:
	pop	bx				; ��������������� ������������� �����
	mov	cl,buffer_len
	mov	ah,3Fh
	int	21h				; ������ �����
	jc	short errreadf			; ���� ��������� �� �������, �� ������

	test	ax,ax
	jz	short closefile			; ���� ������ �� ���������, ������� � ��������
	
	push	bx				; ��������� ������������� �����
	mov	cx,ax				; � CX - ���������� ����������� ����
	mov	bx,1
	mov	ah,40h
	int	21h				; ����� ���� ��� ��������� (�������� ������ � DS ��� � DX)
	jmp	short readloop

erropf:
 	mov	dx,offset erropfmsg
	mov	ah,09h
	int	21h
	jmp	short exit

errreadf:
 	mov	dx,offset errreadfmsg
	mov	ah,09h
	int	21h

closefile:
	mov	ah,3Eh
 	int	21h				; �������� ����� (������������� ����� ������ ��� ���� � BX)

exit:
	mov	ah,4Ch
	int	21h
cseg	ends
end	start