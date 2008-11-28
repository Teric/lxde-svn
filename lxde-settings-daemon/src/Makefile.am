bin_PROGRAMS=lxde-settings-daemon

lxde_settings_daemon_CFLAGS = $(GLIB_CFLAGS) \
                   -DPACKAGE_DATA_DIR=\""$(datadir)/lxde"\" \
                   -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale"\"

lxde_settings_daemon_LDADD = $(GLIB_LIBS) -lX11

lxde_settings_daemon_SOURCES = \
	lxde-settings-daemon.c \
	xsettings-common.h \
	xsettings-common.c \
	xsettings-manager.h \
	xsettings-manager.c \
	xutils.h \
	xutils.c

