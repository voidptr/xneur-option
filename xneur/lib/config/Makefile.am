INCLUDES = -DPACKAGE_SYSCONFDIR_DIR=\""@sysconfdir@/xneur"\" -DPACKAGE_SHAREDIR_DIR=\""@datarootdir@/xneur"\"

lib_LTLIBRARIES = libxnconfig.la

xnincludedir = $(includedir)/xneur
xninclude_HEADERS = xnconfig.h

noinst_HEADERS =		\
	xnconfig_files.h	\
	xnconfig_memory.h

libxnconfig_la_SOURCES =	\
	xnconfig_files.c	\
	xnconfig.c		\
	xnconfig_memory.c

libxnconfig_la_CFLAGS = -I@top_srcdir@/lib/misc @DEFAULT_CFLAGS@
libxnconfig_la_LDFLAGS = -version-info 9:5:0
libxnconfig_la_LIBADD = ../misc/libxnmisc.la

EXTRA_DIST = $(config_DATA)

install-data-local:
	$(LDCONFIG)
