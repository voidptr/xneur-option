noinst_LTLIBRARIES = libxnmisc.la

xnconfigdir = $(includedir)/xneur
xnconfig_HEADERS = list_char.h

noinst_HEADERS =		\
	colors.h		\
	debug.h			\
	log.h			\
	text.h			\
	types.h			\
	regexp.h

libxnmisc_la_SOURCES =		\
	debug.c			\
	list_char.c		\
	log.c			\
	text.c			\
	regexp.c

libxnmisc_la_CFLAGS = @PCRE_CFLAGS@ @DEFAULT_CFLAGS@
libxnmisc_la_LDFLAGS = -static @PCRE_LIBS@
