SUBDIRS=po

bin_PROGRAMS=lxde-obmenu

INCLUDES= \
	$(all_includes) \
	-I$(top_srcdir) \
	$(PACKAGE_CFLAGS) \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"


lxde_obmenu_SOURCES= \
	lxde-obmenu.c

lxde_obmenu_LDADD= \
	$(PACKAGE_LIBS)
