
dist_man_MANS = lxde-settings-daemon.1

EXTRA_DIST = lxde-settings-daemon.sgml

dist-hook: lxde-settings-daemon.1

lxde-settings-daemon.1: lxde-settings-daemon.sgml
	(test -f "`which docbook-to-man`" && docbook-to-man $< > $@) || \
	(test -f "`which db2man`" && db2man $< > $@) || \
	(test -f "`which docbook2man`" && docbook2man $< > $@)

