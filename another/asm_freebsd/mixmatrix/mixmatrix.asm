	GLOBAL main
	EXTERN printf

	SECTION .data

	msize		dd	4

	mxa		dw	1,2,3,4,	\
				5,6,7,8,	\
				9,1,2,3,	\
				4,5,6,7

	mxb		dw	9,8,7,6,	\
				5,4,3,2,	\
				1,9,8,7,	\
				6,5,4,3

	mxtmp times 16	dw	0

	mxc times 16	dd	0

	fshort		db	"%hd | ", 0
	fint		db	"%3d | ", 0

	newline		db	10, 0
	column		db	"| ", 0
	matrixa		db	"Matrix A:", 10, 0
	matrixb		db	"Matrix B:", 10, 0
	matrixc		db	"Result matrix:", 10, 0

	SECTION .text

main:
	push	ebp
	push	esi
	push	edi

	mov	ebp, esp

	push	2
	push	fshort
	push	matrixa
	push	mxa
	call	_print_matrix
	add	esp, 16

	push	2
	push	fshort
	push	matrixb
	push	mxb
	call	_print_matrix
	add	esp, 16

	push	mxc
	push	mxb
	push	mxa
	call	_mul_matrix
	add	esp, 12

	push	4
	push	fint
	push	matrixc
	push	mxc
	call	_print_matrix
	add	esp, 16

	mov	esp, ebp
	pop	edi
	pop	esi
	pop	ebp
	ret

_do_transp:				; Трансцендирование матрицы
	mov	eax, [esp + 4]		; Помещаем в eax указатель на матрицу

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

	push	word [eax + edx * 2]

	mov	eax, [esp + 8 + 4 * 4 + 1 * 2]

	pop	word [eax + ecx * 2]

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

	ret

_mul_matrix:				; Процедура умножения матриц
	mov	ebx, [esp + 8]

	push	mxtmp			; Временная матрица
	push	ebx
	call	_do_transp		; Переворачиваем матрицу
	add	esp, 8

	mov	dword [esp + 8], mxtmp	; mxa = mxtmp

	mov	eax, [esp + 4]		; eax = mxa
	mov	ebx, [esp + 8]		; ebx = mxb
	mov	ecx, [esp + 12]		; ecx = mxc

	add	esp, -8

	mov	[esp + 4], dword 0	; Cчётчик строк

nextrow:
	mov	[esp], dword 0		; Cчётчик столбцов

nextcolumn:				; Следующий столбец
	xor	edx, edx		; Обнуляем edx (счётчик элементов в строке)

	pxor	mm0, mm0
	pxor	mm1, mm1
	pxor	mm7, mm7

nextelement:				; Следующие элементы
	movq	mm0, [eax + edx * 8]	; Загрузка 8-ми байт из матрицы a
	movq	mm1, [ebx + edx * 8]	; Загрузка 8-ми байт из матрицы b

	pmaddwd	mm0, mm1
	paddd	mm7, mm0

	add	edx, 4			; Увеличиваем счётчик ЭЛЕМЕНТОВ (у нас два байта на элемент) на 4
	cmp	edx, dword [msize]	; Сравниваем с размером
	jne	short nextelement	; И если элементы ещё не кончились, то продолжаем дальше

	movq	mm0, mm7
	psrlq	mm7, 32
	paddd	mm7, mm0

	mov	edx, [esp]		; Восстанавливаем счётчик столбцов

	movd	[ecx], mm7
	add	ecx, 4			; Переходим к следующему элементу в результирующей матрице

	inc	edx
	cmp	edx, dword [msize]
	je	short column_done	; И если столбцы кончились, то переходим к следующей строке

	mov	[esp], edx		; Сохраняем счётчик столбцов

	mov	edx, dword [msize]
	lea	ebx, [ebx + edx * 2]	; Переходим к следующему столбцу

	jmp	short nextcolumn

column_done:
	mov	edx, [esp + 4]		; Восстанавливаем счётчик строк
	inc	edx
	cmp	edx, dword [msize]
	je	short done		; И если строки кончились, то выходим

	mov	[esp + 4], edx		; Сохраняем счётчик строк

	mov	edx, dword [msize]
	lea	eax, [eax + edx * 2]	; Переходим к следующей строке
	mov	ebx, [esp + 16]		; Восстанавливаем начальный столбец

	jmp	short nextrow

done:
	add	esp, 8

	ret

_print_matrix:				; Процедура вывода матрицы
	push	dword [esp + 8]		; Заголовок для вывода
	call	printf
	add	esp, 4

	mov	eax, [esp + 4]		; Матрица для вывода

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

	push	dword [eax]
	push	dword [esp + 12 + 3 * 4 + 4]
	call	printf
	add	esp, 8

	pop	eax
	pop	edx
	pop	ecx

	add	eax, [esp + 16]
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

	ret
