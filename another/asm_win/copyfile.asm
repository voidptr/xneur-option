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
	int	21h				; Требование ввода

	mov	dx,offset rn
	mov	ah,09h
	int	21h				; Вставка \r\n

	mov	bl,input_len
	test	bl,bl
	jz	short again1			; Если не ввели ничего, то обратно

	mov	ds:buffer[bx],0			; Дополняем буфер-источник нулем

	mov	al,10000100b			; Режим доступа
	mov	dx,offset buffer
	mov	ah,3Dh
	int	21h				; Открытие файла
	jc	short erropf			; Если открыть не удалось, то ошибка

	mov	src_fid,ax			; AX в src_fid

	jmp	short createf

again2:
 	mov	dx,offset rn
	mov	ah,09h
	int	21h				; Вставка \r\n

createf:
	mov	dx,offset edstfname
	mov	ah,09h
	int	21h				; Please enter destination filename

	mov	dx,offset buffer_len
	mov	ah,0Ah
	int	21h				; Требование ввода

	mov	bl,input_len
	test	bl,bl
	jz	short again2			; Если не ввели ничего, то обратно

	mov	ds:buffer[bx],0			; Дополняем буфер-источник нулем

	xor	cx,cx				; Режим доступа
	mov	dx,offset buffer
	mov	ah,5Bh
	int	21h				; Создание файла
	jc	short errcrf			; Если открыть не удалось, то ошибка

	mov	dst_fid,ax			; AX в dst_fid

	mov	dx,offset buffer		; В DX - смещение буфера в DS

readloop:
	mov	bx,src_fid			; В BX - идентификатор файла-источника
	mov	cl,buffer_len
	mov	ah,3Fh
	int	21h				; Чтение файла
	jc	short errreadf			; Если прочитать не удалось, то ошибка

	test	ax,ax
	jz	short closeall			; Если ничего не прочитано, переход к закрытию
	
	mov	bx,dst_fid			; В BX - идентификатор файла для записи
	mov	cx,ax				; В CX - количество прочитанных байт
	mov	ah,40h
	int	21h				; Запись прочитанного текста в файл (смещение буфера в DS ещё в DX)
	jc	short errwritef			; Если записать не удалось, то ошибка

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
 	int	21h				; Закрытие файла для записи

closereadfile:
	mov	bx,src_fid
	mov	ah,3Eh
 	int	21h				; Закрытие файла-источника

exit:
	mov	ah,4Ch
	int	21h
cseg	ends
end	start