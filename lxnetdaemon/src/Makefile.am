## Process this file with automake to produce Makefile.in

bin_PROGRAMS = lxnetdaemon

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxnetdaemon_SOURCES = \
	lxnetdaemon.h \
	lxnetdaemon.c

lxnetdaemon_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)
