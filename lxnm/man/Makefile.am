
dist_man_MANS = lxnm.1

EXTRA_DIST = lxnm.sgml

dist-hook: lxnm.1

lxnm.1: lxnm.sgml
	docbook-to-man $< > $@
