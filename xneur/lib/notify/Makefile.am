noinst_LTLIBRARIES = libxnnotify.la

libxnnotify_la_SOURCES =	\
	notify.c		\
	notify.h		\
	sound.c			\
	sound.h			\
	osd.c			\
	osd.h

libxnnotify_la_CFLAGS = -I@top_srcdir@/lib/config -I@top_srcdir@/lib/misc @X11_CFLAGS@ @FREEALUT_CFLAGS@ @GSTREAMER_CFLAGS@ @DEFAULT_CFLAGS@
libxnnotify_la_LDFLAGS = -static @X11_LIBS@ @FREEALUT_LIBS@ @GSTREAMER_LIBS@  @APLAY_LIBS@ @XOSD_LIBS@

