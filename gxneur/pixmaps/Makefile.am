icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = gxneur.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = gxneur.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
	echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

pixmapsdir = @datarootdir@/gxneur/pixmaps
pixmaps_DATA =			\
	am.png			\
	bg.png			\
	by.png			\
	cz.png			\
	de.png			\
	en.png			\
	es.png			\
	ee.png			\
	fr.png			\
	ge.png			\
	gr.png			\
	gxneur.png		\
	kz.png			\
	lt.png			\
	lv.png			\
	pl.png			\
	ro.png			\
	ru.png			\
	ua.png			\
	us.png			\
	uz.png			\
	x3.png

EXTRA_DIST = $(pixmaps_DATA)
