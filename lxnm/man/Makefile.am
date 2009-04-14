
dist_man_MANS = lxnm.1

EXTRA_DIST = lxnm.sgml

dist-hook: lxnm.1

lxnm.1: lxnm.sgml
	(test -f "`which docbook-to-man`" && docbook-to-man $< > $@) || \
	(test -f "`which db2man`" && db2man $< > $@) || \
	(test -f "`which docbook2man`" && docbook2man $< > $@)
