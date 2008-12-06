SUBDIRS = src po

EXTRA_DIST = \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

nstall-data-local:
	cp -fr $(srcdir)/data/* $(DESTDIR)/${datadir}

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.la
	rm -f $(DESTDIR)$(libdir)/lxpanel/plugins/*.a
