NULL=

SUBDIRS = src po

ui_in_files= \
	data/lxinput.glade \
	$(NULL)
uidir=$(datadir)/lxinput
ui_DATA = $(ui_in_files:.glade=.ui)

desktopfiledir=$(datadir)/applications
desktopfile_DATA=data/lxinput.desktop

EXTRA_DIST = \
	$(ui_DATA) \
	$(desktopfile_DATA) \
	$(NULL)

CLEANFILES = \
	$(ui_DATA) \
	$(NULL)

%.ui: %.glade
	cp $< $@
	$(top_builddir)/src/xml-purge $@
