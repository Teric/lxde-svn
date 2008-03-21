SUBDIRS = src utils data

EXTRA_DIST = \
	autogen.sh \
	mkinstalldirs \
	autogen.sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

install-exec-hook:
	for script in $(datadir)/lxnm/*.sh; do \
	    chmod +x $$script; \
	done
