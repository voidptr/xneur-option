sseg	segment	dword	stack
	db	100	dup(?)
sseg ends

dseg	segment	dword
	esrcfname	db	"Please enter source filename: $"
	edstfname	db	"Please enter destination filename: $"

	erropfmsg	db	"Error opening file$"
	errcrfmsg	db	"Error creating file$"

	errrdfmsg	db	"Error reading file$"
	errwrfmsg	db	"Error writting file$"

	rn		db	0Dh,0Ah,'$'

	buffer_len	db	253
	input_len	db	?
	buffer		db	253	dup(?)

	src_fid		dw	?
	dst_fid		dw	?
dseg ends

cseg	segment	dword
	assume	cs:cseg,ds:dseg,ss:sseg
start:
	mov	ax,dseg
	mov	ds,ax

	xor	bx,bx

again1:
	mov	dx,offset esrcfname
	mov	ah,09h
	int	21h				; Please enter source filename

	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; ���������� �����

	mov	dx,offset rn
	mov	ah,09h
	int	21h				; ������� \r\n

	mov	bl,input_len
	test	bl,bl
	jz	short again1			; ���� �� ����� ������, �� �������

	mov	ds:buffer[bx],0			; ��������� �����-�������� �����

	mov	al,10000100b			; ����� �������
	mov	dx,offset buffer
	mov	ah,3Dh
	int	21h				; �������� �����
	jc	short erropf			; ���� ������� �� �������, �� ������

	mov	src_fid,ax			; AX � src_fid

	jmp	short createf

again2:
 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; ������� \r\n

createf:
	mov	dx,offset edstfname
	mov	ah,09h
	int	21h				; Please enter destination filename

	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; ���������� �����

	mov	bl,input_len
	test	bl,bl
	jz	short again2			; ���� �� ����� ������, �� �������

	mov	ds:buffer[bx],0			; ��������� �����-�������� �����

	xor	cx,cx				; ����� �������
	mov	dx,offset buffer
	mov	ah,5Bh
	int	21h				; �������� �����
	jc	short errcrf			; ���� ������� �� �������, �� ������

	mov	dst_fid,ax			; AX � dst_fid

	mov	dx,offset buffer		; � DX - �������� ������ � DS

readloop:
	mov	bx,src_fid			; � BX - ������������� �����-���������
	mov	cl,buffer_len
	mov	ah,3Fh
	int	21h				; ������ �����
	jc	short errreadf			; ���� ��������� �� �������, �� ������

	test	ax,ax
	jz	short closeall			; ���� ������ �� ���������, ������� � ��������
	
	mov	bx,dst_fid			; � BX - ������������� ����� ��� ������
	mov	cx,ax				; � CX - ���������� ����������� ����
	mov	ah,40h
	int	21h				; ������ ������������ ������ � ���� (�������� ������ � DS ��� � DX)
	jc	short errwritef			; ���� �������� �� �������, �� ������

	jmp	short readloop

erropf:
 	mov	dx,offset erropfmsg
	mov	ah,09h
	int	21h
	jmp	short exit

errcrf:
 	mov	dx,offset errcrfmsg
	mov	ah,09h
	int	21h
	jmp	short closereadfile

errreadf:
 	mov	dx,offset errrdfmsg
	mov	ah,09h
	int	21h
	jmp	short closeall
	
errwritef:
 	mov	dx,offset errwrfmsg
	mov	ah,09h
	int	21h

closeall:
	mov	bx,dst_fid
	mov	ah,3Eh
 	int	21h				; �������� ����� ��� ������

closereadfile:
	mov	bx,src_fid
	mov	ah,3Eh
 	int	21h				; �������� �����-���������

exit:
	mov	ah,4Ch
	int	21h
cseg	ends
end	start