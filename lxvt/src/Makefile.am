bin_PROGRAMS=lxvt

INCLUDES=\
    -I$(top_srcdir) \
    $(PACKAGE_CFLAGS) \
    -DPACKAGE_LOCALE_DIR=\"$(datadir)/locale\"

lxvt_SOURCES=\
    lxvt.c \
    window.c window.h \
    term.c term.h

lxvt_LDADD=\
    $(PACKAGE_LIBS) \
    $(INTLLIBS)

#EXTRA_DIST =
