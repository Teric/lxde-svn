
dist_man_MANS = lxde-settings.1

EXTRA_DIST = lxde-settings.sgml

dist-hook: lxde-settings.1

lxde-settings.1: lxde-settings.sgml
	(test -f "`which docbook-to-man`" && docbook-to-man $< > $@) || \
	(test -f "`which db2man`" && db2man $< > $@) || \
	(test -f "`which docbook2man`" && docbook2man $< > $@)

