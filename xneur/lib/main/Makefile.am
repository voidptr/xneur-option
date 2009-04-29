noinst_LTLIBRARIES = libxnx.la

libxnx_la_SOURCES =		\
	bind_table.c		\
	bind_table.h		\
	event.c			\
	event.h			\
	focus.c			\
	focus.h			\
	selection.c		\
	selection.h		\
	switchlang.c		\
	switchlang.h		\
	typographics.c		\
	typographics.h		\
	xdefines.h		\
	xprogram.c		\
	xprogram.h		\
	xstring.c		\
	xstring.h		\
	xutils.c		\
	xutils.h		\
	xwindow.c		\
	xwindow.h		\
	xkeymap.c		\
	xkeymap.h

libxnx_la_CFLAGS = -I@top_srcdir@/lib/config -I@top_srcdir@/lib/misc  -I@top_srcdir@/lib/notify -I@top_srcdir@/lib/ai @X11_CFLAGS@ @DEFAULT_CFLAGS@
libxnx_la_LDFLAGS = -static @X11_LIBS@
