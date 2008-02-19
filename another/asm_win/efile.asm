sseg	segment	dword	stack
	db	100	dup(?)
sseg ends

dseg	segment	dword
	edstfname	db	"Please enter destination filename: $"
	etext		db	"Please enter text: ",0Dh,0Ah,'$'
	errcrfmsg	db	"Error creating file$"
	errwrfmsg	db	"Error writting file$"

	rn		db	0Dh,0Ah,'$'

	buffer_len	db	253
	input_len	db	?
	buffer		db	253	dup(?)
dseg ends

cseg	segment	dword
	assume	cs:cseg,ds:dseg,ss:sseg
start:
	mov	ax,dseg
	mov	ds,ax

	xor	bx,bx

again:
	mov	dx,offset edstfname
	mov	ah,09h
	int	21h				; Please enter destination filename

	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; ���������� �����

 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; ������� \r\n

	mov	bl,input_len
	test	bl,bl
	jz	short again			; ���� �� ����� ������, �� �������

	mov	ds:buffer[bx],0			; ��������� �����-�������� �����

	xor	cx,cx				; ����� �������
	mov	dx,offset buffer
	mov	ah,5Bh
	int	21h				; �������� �����
	jc	short errcrf			; ���� ������� �� �������, �� ������

	mov	bx,ax				; ������������� ����� � BX

 	mov	dx,offset etext
	mov	ah,09h
	int	21h				; Please enter text

readloop:
	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; ���������� �����

	mov	cl,input_len			; � CX - ���������� ����������� ����
	test	cl,cl
	jz	short pastern			; ���� �� ����� ������, �� ������ ��������� \r\n

	cmp	ds:buffer[0],04h		; ���������� ������ ��������� ������ � Ctrl+D
	je	short closeall			; ���� ����� Ctrl+D, �� ��������� ���� � �������

	mov	dx,offset buffer		; � DX - �������� ������ � DS
	mov	ah,40h
	int	21h				; ������ ��������� ������ � ���� (������������� ����� �� ��� � BX)
	jc	short errwritef			; ���� �������� �� �������, �� ������

pastern:
 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; ������� \r\n

	mov	cl,2				; � CX - ������ ����� ��� ������ (\r\n - ��� �������)
	mov	ah,40h
	int	21h				; ������ ��������� ������ � ���� (������������� ����� �� ��� � BX, �������� ������ ��� ��� � DX)
	jc	errwritef			; ���� �������� �� �������, �� ������

	jmp	short readloop

errcrf:
 	mov	dx,offset errcrfmsg
	mov	ah,09h
	int	21h
	jmp	short exit

errwritef:
 	mov	dx,offset errwrfmsg
	mov	ah,09h
	int	21h

closeall:
	mov	ah,3Eh
 	int	21h				; �������� ����� ��� ������ (������������� ����� �� ��� � BX)

exit:
	mov	ah,4Ch
	int	21h
cseg	ends
end	start