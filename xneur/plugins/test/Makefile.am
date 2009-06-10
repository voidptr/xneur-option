## Process this file with automake to produce Makefile.in

INCLUDES = -DPACKAGE_SYSCONFDIR_DIR=\""@sysconfdir@/xneur"\" -DPACKAGE_SHAREDIR_DIR=\""@datarootdir@/xneur"\"

lib_LTLIBRARIES = libxntest.la

libxntest_la_SOURCES = \
	test.c

libxntest_la_CFLAGS = -I@top_srcdir@/lib/config @DEFAULT_CFLAGS@
libxntest_la_LDFLAGS = 
	 
libxntest_la_LIBADD = 
