INCLUDES = 	-DPACKAGE_PIXMAPS_DIR=\""@datadir@/gxneur/pixmaps"\" 	\
		-DLOCALEDIR=\""@localedir@"\"				\
		-DPACKAGE_GLADE_FILE_DIR=\""@datadir@/gxneur/glade"\"	\
		-DXNEUR_PLUGIN_DIR=\""@libdir@/xneur"\"

bin_PROGRAMS = gxneur

gxneur_SOURCES =		\
	callbacks.c		\
	callbacks.h		\
	clock.c			\
	clock.h			\
	main.c			\
	misc.c			\
	misc.h			\
	support.c		\
	support.h		\
	tray_widget.c		\
	tray_widget.h		\
	trayicon.c		\
	trayicon.h		\
	xkb.c			\
	xkb.h

gxneur_CFLAGS = @GDK_CFLAGS@ @GTK_CFLAGS@ @XNEUR_CFLAGS@ @GLADE_CFLAGS@ @DEFAULT_CFLAGS@ @XNEURLIB_CFLAGS@
gxneur_LDFLAGS = @GDK_LIBS@ @GTK_LIBS@ @XNEUR_LIBS@ @GLADE_LIBS@ @XNEURLIB_LIBS@
