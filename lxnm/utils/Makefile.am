## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = lxnetctl

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxnetctl_SOURCES = \
	lxnetctl.c

lxnetctl_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)
