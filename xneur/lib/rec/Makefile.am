noinst_LTLIBRARIES = libxnrec.la

libxnrec_la_SOURCES =	\
	rec.c		\
	rec.h		

libxnrec_la_CFLAGS = -I@top_srcdir@/lib/config -I@top_srcdir@/lib/misc -I@top_srcdir@/lib/lib 		\
						@X11_CFLAGS@ @DEFAULT_CFLAGS@ 	

libxnrec_la_LDFLAGS = -static @X11_LIBS@ @ADDITIONAL_LIBS@

