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
	int	21h				; Требование ввода
	
 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; Вставка \r\n
	
	xor     bx,bx
	mov	bl,input_len
	test	bl,bl
	jz	short again			; Если не ввели ничего, то обратно
	
	mov	ds:buffer[bx],0			; Дополняем буфер нулем

	mov	al,10000100b			; Режим доступа
	mov	dx,offset buffer
	mov	ah,3Dh
	int	21h				; Открытие файла
	jc	short erropf			; Если открыть не удалось, то ошибка

	mov	dx,offset buffer		; В DX - смещение буфера в DS
	push	ax

readloop:
	pop	bx				; Восстанавливаем идентификатор файла
	mov	cl,buffer_len
	mov	ah,3Fh
	int	21h				; Чтение файла
	jc	short errreadf			; Если прочитать не удалось, то ошибка

	test	ax,ax
	jz	short closefile			; Если ничего не прочитано, переход к закрытию
	
	push	bx				; Сохраняем идентификатор файла
	mov	cx,ax				; В CX - количество прочитанных байт
	mov	bx,1
	mov	ah,40h
	int	21h				; Вывод того что прочитано (смещение буфера в DS ещё в DX)
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
 	int	21h				; Закрытие файла (идентификатор файла должен ещё быть в BX)

exit:
	mov	ah,4Ch
	int	21h
cseg	ends
end	start