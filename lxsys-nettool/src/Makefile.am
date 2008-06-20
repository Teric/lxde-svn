## Process this file with automake to produce Makefile.in

sbin_PROGRAMS = lxsys-nettool

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxsys_nettool_SOURCES = \
	lxsys-nettool.h \
	lxsys-nettool.c

lxsys_nettool_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)
