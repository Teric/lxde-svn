## Process this file with automake to produce Makefile.in

# set the include path found by configure
INCLUDES = $(all_includes) \
	@PACKAGE_CFLAGS@ \
	-I$(top_srcdir) \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

bin_PROGRAMS = gpicview
gpicview_SOURCES = \
	gpicview.c \
	image-list.c \
	main-win.c \
	image-view.c \
	working-area.c \
	ptk-menu.c \
	file-dlgs.c \
	jpeg-tran.c \
	tran-supp.c \
	pref.c \
	jhead.c \
	exif.c \
	jpgfile.c 

# the library search path.
# use -nodefaultlibs to prevent linking libstdc++
gpicview_LDADD = @PACKAGE_LIBS@

noinst_HEADERS = \
	glib-mem.h \
	image-list.h \
	main-win.h \
	image-view.h \
	working-area.h \
	ptk-menu.h \
	file-dlgs.h \
	jpeg-tran.h \
	tran-supp.h \
	pref.h \
	jhead.h

noinst_PROGRAMS=xml-purge
xml_purge_SOURCES=xml-purge.c
xml_purge_CFLAGS=@PACKAGE_CFLAGS@
xml_purge_LDADD=@PACKAGE_LIBS@

