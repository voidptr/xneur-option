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

_do_transp:				; Трансцендирование матрицы
	push	ebp
	mov	ebp, esp

	mov	eax, [ebp + 4 + 4 * 1]	; Помещаем в eax указатель на исходную матрицу

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

	push	dword [eax + edx * 4]	; Ячейка mxb[edx]

	mov	eax, [ebp + 4 + 4 * 2]	; Помещаем в eax указатель на результирующую матрицу

	pop	dword [eax + ecx * 4]	; Ячейка mxt[ecx]

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

_mul_matrix:				; Процедура умножения матриц
	push	ebp
	mov	ebp, esp

	push	mxt			; Временная матрица
	push	dword [ebp + 4 + 4 * 2]	; Матрица mxb
	call	_do_transp		; Переворачиваем матрицу (mxb в mxt)
	add	esp, 8

	mov	dword [ebp + 4 + 4 * 2], mxt

	mov	eax, [ebp + 4 + 4 * 1]	; eax = mxa
	mov	ebx, [ebp + 4 + 4 * 2]	; ebx = mxb
	mov	ecx, [ebp + 4 + 4 * 3]	; ecx = mxc

	add	esp, -8			; Резервируем место в стеке для двух элементов

	mov	dword [esp + 4], 0	; Счётчик строк

nextrow:
	mov	dword [esp], 0		; Cчётчик столбцов

nextcolumn:				; Следующий столбец
	xor	edx, edx		; Обнуляем edx (счётчик элементов в строке)

	xorps	xmm0, xmm0
	xorps	xmm1, xmm1
	xorps	xmm7, xmm7

nextelement:				; Следующие элементы
	shl	edx, 4			; Умножаем edx на 16

	movups	xmm0, [eax + edx]	; Загрузка 16-ти байт из матрицы a
	movups	xmm1, [ebx + edx]	; Загрузка 16-ти байт из матрицы b

	shr	edx, 4

	mulps	xmm0, xmm1
	addps	xmm7, xmm0
	
	add	edx, 4			; Увеличиваем счётчик ЭЛЕМЕНТОВ на 4
	cmp	edx, dword [msize]	; Сравниваем с размером
	jne	short nextelement	; И если элементы ещё не кончились, то продолжаем дальше

	movaps	xmm0, xmm7
	shufps	xmm0, xmm0, 10110001b
	addps	xmm7, xmm0

	movaps	xmm0, xmm7
	shufps	xmm0, xmm0, 01001110b
	addps	xmm7, xmm0

	mov	edx, [esp]		; Восстанавливаем счётчик столбцов

	movss	[ecx], xmm7

	add	ecx, 4			; Переходим к следующему элементу в результирующей матрице

	inc	edx
	cmp	edx, dword [msize]
	je	short column_done	; И если столбцы кончились, то переходим к следующей строке

	mov	[esp], edx		; Сохраняем счётчик столбцов

	mov	edx, dword [msize]
	lea	ebx, [ebx + edx * 4]	; Переходим к следующему столбцу

	jmp	short nextcolumn

column_done:
	mov	edx, [esp + 4]		; Восстанавливаем счётчик строк
	inc	edx
	cmp	edx, dword [msize]
	je	short done		; И если строки кончились, то выходим

	mov	[esp + 4], edx		; Сохраняем счётчик строк

	mov	edx, dword [msize]
	lea	eax, [eax + edx * 4]	; Переходим к следующей строке
	mov	ebx, [ebp + 4 + 4 * 2]	; Восстанавливаем начальный столбец

	jmp	nextrow

done:
	add	esp, 8

	mov	esp, ebp
	pop	ebp

	ret

_print_matrix:				; Процедура вывода матрицы
	push	ebp
	mov	ebp, esp

	push	dword [ebp + 4 + 4 * 2]	; Заголовок для вывода
	call	printf
	add	esp, 4

	mov	eax, [ebp + 4 + 4 * 1]	; Матрица для вывода

	xor	edx, edx		; Счётчик высоты

pcolumn:
	xor	ecx, ecx		; Счётчик длины

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
	push	ffloat			; Формат для вывода
	call	printf
	add	esp, 12

	pop	eax
	pop	edx
	pop	ecx

	add	eax, 4			; На следующую ячейку для вывода
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
