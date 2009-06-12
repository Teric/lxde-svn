SUBDIRS = src po

menudir =  $(datadir)/lxpanel/gmenu
menu_DATA = data/menu.png \
            data/menu-button.png \
            data/gtkrc

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
    $(menu_DATA)

install-exec-hook:
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.la
	rm -f $(DESTDIR)$(LXPANEL_LIBDIR)/plugins/*.a
