INCLUDES = \
	-I. \
	-I$(top_srcdir)/src \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

module_LTLIBRARIES = gmenu.la

moduledir = $(libdir)/lxpanel/plugins

gmenu_la_SOURCES = gmenu-dlg.h \
                   gmenu-dlg.c \
                   menu.h \
                   menu.c \
                   gmenu.c

gmenu_la_LIBADD = $(PACKAGE_LIBS)

gmenu_la_LDFLAGS = \
	-module \
	@LXPANEL_MODULE@
