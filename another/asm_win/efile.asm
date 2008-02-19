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
	int	21h				; Требование ввода

 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; Вставка \r\n

	mov	bl,input_len
	test	bl,bl
	jz	short again			; Если не ввели ничего, то обратно

	mov	ds:buffer[bx],0			; Дополняем буфер-источник нулем

	xor	cx,cx				; Режим доступа
	mov	dx,offset buffer
	mov	ah,5Bh
	int	21h				; Создание файла
	jc	short errcrf			; Если открыть не удалось, то ошибка

	mov	bx,ax				; Идентификатор файла в BX

 	mov	dx,offset etext
	mov	ah,09h
	int	21h				; Please enter text

readloop:
	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; Требование ввода

	mov	cl,input_len			; В CX - количество прочитанных байт
	test	cl,cl
	jz	short pastern			; Если не ввели ничего, то просто вставляем \r\n

	cmp	ds:buffer[0],04h		; Сравниваем первый введенный символ с Ctrl+D
	je	short closeall			; Если ввели Ctrl+D, то закрываем файл и выходим

	mov	dx,offset buffer		; В DX - смещение буфера в DS
	mov	ah,40h
	int	21h				; Запись введённого текста в файл (идентификатор файла всё ещё в BX)
	jc	short errwritef			; Если записать не удалось, то ошибка

pastern:
 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; Вставка \r\n

	mov	cl,2				; В CX - размер текса для записи (\r\n - два символа)
	mov	ah,40h
	int	21h				; Запись введённого текста в файл (идентификатор файла всё ещё в BX, смещение буфера все ещё в DX)
	jc	errwritef			; Если записать не удалось, то ошибка

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
 	int	21h				; Закрытие файла для записи (идентификатор файла всё ещё в BX)

exit:
	mov	ah,4Ch
	int	21h
cseg	ends
end	start