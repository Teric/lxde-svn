## Process this file with automake to produce Makefile.in

sbin_PROGRAMS = lxnm

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

lxnm_SOURCES = \
	handler.c \
	handler.h \
	wireless.c \
	wireless.h \
	lxnm.h \
	lxnm.c

lxnm_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS) \
		-liw
