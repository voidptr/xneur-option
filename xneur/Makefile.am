ACLOCAL_AMFLAGS = -I m4

SUBDIRS =			\
	lib			\
	src			\
	share			\
	etc

AUTOMAKE_OPTIONS =		\
	1.9			\
	dist-bzip2

noinst_DATA =			\
	README			\
	COPYING			\
	AUTHORS			\
	INSTALL			\
	NEWS			\
	TODO			\
	ChangeLog

pkgconfigdir = @libdir@/pkgconfig
pkgconfig_DATA = xnconfig.pc

EXTRA_DIST = $(noinst_DATA) $(pkgconfig_DATA) xnconfig.pc.in xneur.1 xneurrc.5
man_MANS = xneur.1 xneurrc.5
