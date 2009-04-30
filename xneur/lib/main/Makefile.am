noinst_LTLIBRARIES = libxnx.la

libxnx_la_SOURCES =		\
	bind_table.c		\
	bind_table.h		\
	buffer.c		\
	buffer.h		\
	event.c			\
	event.h			\
	focus.c			\
	focus.h			\
	keymap.c		\
	keymap.h		\
	program.c		\
	program.h		\
	selection.c		\
	selection.h		\
	switchlang.c		\
	switchlang.h		\
	utils.c			\
	utils.h			\
	window.c		\
	window.h		\
	defines.h


libxnx_la_CFLAGS = -I@top_srcdir@/lib/config -I@top_srcdir@/lib/misc  -I@top_srcdir@/lib/notify -I@top_srcdir@/lib/ai @X11_CFLAGS@ @DEFAULT_CFLAGS@
libxnx_la_LDFLAGS = -static @X11_LIBS@
