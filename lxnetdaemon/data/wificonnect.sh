#!/bin/sh
# <ifname> <essid> <en_type> <password> <bssid>


if [ A"$3" = A"0" ]; then
	if [ -f /var/run/dhclient_${1}.pid ]; then
		kill `cat /var/run/dhclient_${1}.pid`
		rm /var/run/dhclient_${1}.pid
	fi

	# without encryption
	ifconfig $1 up
	if [ A"$5" = A ]; then
		iwconfig $1 essid "$2"
	else
		iwconfig $1 ap $5
	fi
	dhclient $1 -1 -d -pf /var/run/dhclient_${1}.pid
elif [ A"$3" = A"1" ]; then
	# WEP
	echo "network={" > /tmp/lxnd.$1.wep
	echo "     ssid=\"$2\"" >> /tmp/lxnd.$1.wep
	if [ ! A"$5" = A ]; then
		echo "     bssid=\"$5\"" >> /tmp/lxnd.$1.wep
	fi
	echo "     key_mgmt=NONE" >> /tmp/lxnd.$1.wep
	echo -e "     wep_key0=\"$4\"" >> /tmp/lxnd.$1.wep
	echo "}" >> /tmp/lxnd.$1.wep

	wpa_supplicant -BDwext -c/tmp/lxnd.$1.wep -i$1
	dhclient $1 -1 -d -pf /var/run/dhclient_${1}.pid

	rm -fr /tmp/lxnd.$1.wep
elif [ A"$3" = A"2" ]; then
	# WPA-PSK
	echo "ctrl_interface=/var/run/wpa_supplicant" > /tmp/lxnd.$1.wpa-psk
	echo "ctrl_interface_group=0" >> /tmp/lxnd.$1.wpa-psk
	echo "ap_scan=1" >> /tmp/lxnd.$1.wpa-psk
	echo "fast_reauth=1" >> /tmp/lxnd.$1.wpa-psk
	echo "eapol_version=1" >> /tmp/lxnd.$1.wpa-psk

	echo "network={" >> /tmp/lxnd.$1.wpa-psk
	echo "        ssid=\"$2\"" >> /tmp/lxnd.$1.wpa-psk
	if [ ! A"$5" = A ]; then
		echo "     bssid=\"$5\"" >> /tmp/lxnd.$1.wpa-psk
	fi
	echo "        psk=\"$4\"" >> /tmp/lxnd.$1.wpa-psk
	echo "        priority=5" >> /tmp/lxnd.$1.wpa-psk
	echo "}" >> /tmp/lxnd.$1.wpa-psk

	wpa_supplicant -BDwext -c/tmp/lxnd.$1.wpa-psk -i$1
	dhclient $1 -1 -d -pf /var/run/dhclient_${1}.pid

	rm -fr /tmp/lxnd.$1.wpa-psk
fi
