SUBDIRS = src po

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.la
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.a
