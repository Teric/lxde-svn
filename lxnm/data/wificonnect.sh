#!/bin/sh
# <ifname> <essid> <en_type> <password> <bssid>

if [ A"$LXNM_WIFI_PROTO" = A"NONE" ]; then
	if [ -f /var/run/dhclient_$LXNM_IFNAME.pid ]; then
		kill `cat /var/run/dhclient_$LXNM_IFNAME.pid`
		rm /var/run/dhclient_$LXNM_IFNAME.pid
	fi

	# without encryption
	ifconfig $LXNM_IFNAME up
	if [ A"$LXNM_WIFI_ESSID" = A ]; then
		iwconfig $LXNM_IFNAME essid "$LXNM_WIFI_ESSID" key off
	else
		iwconfig $LXNM_IFNAME ap "$LXNM_WIFI_APADDR" key off
	fi

	dhclient $LXNM_IFNAME -1 -d -pf /var/run/dhclient_$LXNM_IFNAME.pid
elif [ A"$LXNM_WIFI_PROTO" = A"WEP" ]; then
	if [ -f /var/run/dhclient_$LXNM_IFNAME.pid ]; then
		kill `cat /var/run/dhclient_$LXNM_IFNAME.pid`
		rm /var/run/dhclient_$LXNM_IFNAME.pid
	fi

	# WEP
	ifconfig $LXNM_IFNAME up
	if [ ! A"$LXNM_WIFI_ESSID" = A ]; then
		iwconfig $LXNM_IFNAME essid "$LXNM_WIFI_ESSID" key "s:$LXNM_WIFI_KEY"
	fi

	dhclient $LXNM_IFNAME -1 -d -pf /var/run/dhclient_$LXNM_IFNAME.pid
else
	if [ -f /var/run/dhclient_$LXNM_IFNAME.pid ]; then
		kill `cat /var/run/dhclient_$LXNM_IFNAME.pid`
		rm /var/run/dhclient_$LXNM_IFNAME.pid
	fi

	# start trying to associate with the WPA network using SSID test.
	wpa_supplicant -g/var/run/wpa_supplicant-global -B

	wpa_cli -g/var/run/wpa_supplicant-global interface_remove $LXNM_IFNAME
	wpa_cli -g/var/run/wpa_supplicant-global interface_add $LXNM_IFNAME "" wext /var/run/wpa_supplicant

	wpa_cli -i$LXNM_IFNAME add_network
	wpa_cli -i$LXNM_IFNAME set_network 0 ssid "$LXNM_WIFI_ESSID"
	wpa_cli -i$LXNM_IFNAME set_network 0 key_mgmt "$LXNM_WIFI_KEYMGMT"
	wpa_cli -i$LXNM_IFNAME set_network 0 psk "$LXNM_WIFI_KEY"
	wpa_cli -i$LXNM_IFNAME set_network 0 pairwise "$LXNM_WIFI_PAIRWISE"
	wpa_cli -i$LXNM_IFNAME set_network 0 group "$LXNM_WIFI_GROUP"
	wpa_cli -i$LXNM_IFNAME set_network 0 proto "$LXNM_WIFI_PROTO"
	wpa_cli -i$LXNM_IFNAME enable_network 0

	dhclient $LXNM_IFNAME -1 -d -pf /var/run/dhclient_$LXNM_IFNAME.pid
fi
