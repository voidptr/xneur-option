	GLOBAL main
	EXTERN printf

	SECTION .data

	mxa		dd	1.0, 2.0, 3.0, 4.0,	\
				5.0, 6.0, 7.0, 8.0,	\
				9.0, 1.0, 2.0, 3.0,	\
				4.0, 5.0, 6.0, 7.0

	mxb		dd	9.0, 8.0, 7.0, 6.0,	\
				5.0, 4.0, 3.0, 2.0,	\
				1.0, 9.0, 8.0, 7.0,	\
				6.0, 5.0, 4.0, 3.0

	msize		dd	4

	mxt   times 16	dd	0.0

	mxc   times 16	dd	0.0

	fltmp		dq	0.0

	ffloat		db	"%5.1F | ", 0

	newline		db	10, 0
	column		db	"| ", 0
	matrixa		db	"Matrix A:", 10, 0
	matrixb		db	"Matrix B:", 10, 0
	matrixc		db	"Result matrix:", 10, 0

	SECTION .text

main:
	push	ebp
	mov	ebp, esp

	push	matrixa
	push	mxa
	call	_print_matrix
	add	esp, 8

	push	matrixb
	push	mxb
	call	_print_matrix
	add	esp, 8

	push	mxc
	push	mxb
	push	mxa
	call	_mul_matrix
	add	esp, 12

	push	matrixc
	push	mxc
	call	_print_matrix
	add	esp, 8

	mov	esp, ebp
	pop	ebp

	ret

_do_transp:				; ����������������� �������
	push	ebp
	mov	ebp, esp

	mov	eax, [ebp + 4 + 4 * 1]	; �������� � eax ��������� �� �������� �������

	mov	ebx, dword [msize]	; ebx = size

	xor	ecx, ecx		; ecx = i

tr_inc_i:
	xor	edx, edx		; edx = j

tr_inc_j:
	push	eax
	push	ebx
	push	ecx
	push	edx

	push	ecx

	imul	ecx, ebx
	add	ecx, edx		; i = i * size + j

	imul	edx, ebx
	pop	ebx
	add	edx, ebx		; j = j * size + i

	push	dword [eax + edx * 4]	; ������ mxb[edx]

	mov	eax, [ebp + 4 + 4 * 2]	; �������� � eax ��������� �� �������������� �������

	pop	dword [eax + ecx * 4]	; ������ mxt[ecx]

	pop	edx
	pop	ecx
	pop	ebx
	pop	eax

	inc	edx
	cmp	edx, ebx
	jne	tr_inc_j

	inc	ecx
	cmp	ecx, ebx
	jne	tr_inc_i

	mov	esp, ebp
	pop	ebp

	ret

_mul_matrix:				; ��������� ��������� ������
	push	ebp
	mov	ebp, esp

	push	mxt			; ��������� �������
	push	dword [ebp + 4 + 4 * 2]	; ������� mxb
	call	_do_transp		; �������������� ������� (mxb � mxt)
	add	esp, 8

	mov	dword [ebp + 4 + 4 * 2], mxt

	mov	eax, [ebp + 4 + 4 * 1]	; eax = mxa
	mov	ebx, [ebp + 4 + 4 * 2]	; ebx = mxb
	mov	ecx, [ebp + 4 + 4 * 3]	; ecx = mxc

	add	esp, -8			; ����������� ����� � ����� ��� ���� ���������

	mov	dword [esp + 4], 0	; �ޣ���� �����

nextrow:
	mov	dword [esp], 0		; Cޣ���� ��������

nextcolumn:				; ��������� �������
	xor	edx, edx		; �������� edx (�ޣ���� ��������� � ������)

	xorps	xmm0, xmm0
	xorps	xmm1, xmm1
	xorps	xmm7, xmm7

nextelement:				; ��������� ��������
	shl	edx, 4			; �������� edx �� 16

	movups	xmm0, [eax + edx]	; �������� 16-�� ���� �� ������� a
	movups	xmm1, [ebx + edx]	; �������� 16-�� ���� �� ������� b

	shr	edx, 4

	mulps	xmm0, xmm1
	addps	xmm7, xmm0
	
	add	edx, 4			; ����������� �ޣ���� ��������� �� 4
	cmp	edx, dword [msize]	; ���������� � ��������
	jne	short nextelement	; � ���� �������� �ݣ �� ���������, �� ���������� ������

	movaps	xmm0, xmm7
	shufps	xmm0, xmm0, 10110001b
	addps	xmm7, xmm0

	movaps	xmm0, xmm7
	shufps	xmm0, xmm0, 01001110b
	addps	xmm7, xmm0

	mov	edx, [esp]		; ��������������� �ޣ���� ��������

	movss	[ecx], xmm7

	add	ecx, 4			; ��������� � ���������� �������� � �������������� �������

	inc	edx
	cmp	edx, dword [msize]
	je	short column_done	; � ���� ������� ���������, �� ��������� � ��������� ������

	mov	[esp], edx		; ��������� �ޣ���� ��������

	mov	edx, dword [msize]
	lea	ebx, [ebx + edx * 4]	; ��������� � ���������� �������

	jmp	short nextcolumn

column_done:
	mov	edx, [esp + 4]		; ��������������� �ޣ���� �����
	inc	edx
	cmp	edx, dword [msize]
	je	short done		; � ���� ������ ���������, �� �������

	mov	[esp + 4], edx		; ��������� �ޣ���� �����

	mov	edx, dword [msize]
	lea	eax, [eax + edx * 4]	; ��������� � ��������� ������
	mov	ebx, [ebp + 4 + 4 * 2]	; ��������������� ��������� �������

	jmp	nextrow

done:
	add	esp, 8

	mov	esp, ebp
	pop	ebp

	ret

_print_matrix:				; ��������� ������ �������
	push	ebp
	mov	ebp, esp

	push	dword [ebp + 4 + 4 * 2]	; ��������� ��� ������
	call	printf
	add	esp, 4

	mov	eax, [ebp + 4 + 4 * 1]	; ������� ��� ������

	xor	edx, edx		; �ޣ���� ������

pcolumn:
	xor	ecx, ecx		; �ޣ���� �����

	push	ecx
	push	edx
	push	eax

	push	column
	call	printf
	add	esp, 4

	pop	eax
	pop	edx
	pop	ecx

prow:

	push	ecx
	push	edx
	push	eax

	fld	dword [eax]
	fstp	qword [fltmp]

	push	dword [fltmp + 4]	; 64 bit floating point (bottom)
	push	dword [fltmp]		; 64 bit floating point (top)
	push	ffloat			; ������ ��� ������
	call	printf
	add	esp, 12

	pop	eax
	pop	edx
	pop	ecx

	add	eax, 4			; �� ��������� ������ ��� ������
	inc	ecx
	cmp	ecx, dword [msize]
	jne	short prow

	push	edx
	push	eax

	push	newline
	call	printf
	add	esp, 4

	pop	eax
	pop	edx

	inc	edx
	cmp	edx, dword [msize]
	jne	short pcolumn

	push	newline
	call	printf
	add	esp, 4

	mov	esp, ebp
	pop	ebp

	ret
