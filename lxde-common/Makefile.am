SUBDIRS = icon-theme man

bin_SCRIPTS = startlxde lxde-logout openbox-lxde

# Currently these paths are hardcoded.
xsessiondir=$(xsesspath)
xsession_DATA = LXDE.desktop

defaultsdir=$(sysconfdir)/xdg/lxsession/LXDE
defaults_DATA = autostart config

wallpapersdir=$(datadir)/lxde/wallpapers
wallpapers_DATA= \
	wallpapers/lxde_blue.jpg \
 	wallpapers/lxde_red.jpg \
	wallpapers/lxde_green.jpg

imagesdir=$(datadir)/lxde/images
images_DATA= \
	images/logout-banner.png \
	images/lxde-icon.png

lxpaneldir=$(datadir)/lxpanel/profile/LXDE
lxpanel_DATA=lxpanel/config

lxpanel_paneldir=$(datadir)/lxpanel/profile/LXDE/panels
lxpanel_panel_DATA=lxpanel/panel

openboxdir=$(datadir)/lxde/openbox
openbox_DATA= \
	openbox/rc.xml \
	openbox/menu.xml

pcmanfmdir=$(datadir)/lxde/pcmanfm
pcmanfm_DATA= \
	pcmanfm/main.lxde

EXTRA_DIST = \
	default \
	autostart \
	config \
	LXDE.desktop.in \
	startlxde.in \
	$(wallpapers_DATA) \
	$(images_DATA) \
	icon-theme/COPYING \
	icon-theme/AUTHORS \
	openbox/rc.xml.in \
	openbox/menu.xml \
	openbox-lxde.in \
	pcmanfm/main.lxde.in \
	$(lxpanel_DATA) \
	lxpanel/panel.in

CLEANFILES = $(bin_SCRIPTS)

