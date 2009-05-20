#/bin/sh

LXNM_IFNAME=wlan0
LXNM_ENABLE=0
LXNM_PLUGED=`cat /sys/class/net/${LXNM_IFNAME}/carrier`
LXNM_CONNECTED=0
LXNM_MAC=
LXNM_IPADDR=
LXNM_BROADCAST=
LXNM_NETMASK=
LXNM_DEST=0.0.0.0
LXNM_RX_BYTES=
LXNM_RX_PACKETS=
LXNM_TX_BYTES=
LXNM_TX_PACKETS=
LXNM_ESSID=
LXNM_BSSID=
LXNM_QUALITY=0

while read ARG1 ARG2 ARG3 ARG4 ARG5 ARG6 ARG7; do
	if [ "$ARG1" = "UP" ]; then
		LXNM_ENABLE=1
	fi

	if [ "$ARG4" = "HWaddr" ]; then
		LXNM_MAC=$ARG5
	elif [ "$ARG1" = "inet" ]; then
		LXNM_IPADDR=${ARG2#*:}
		LXNM_BROADCAST=${ARG3#*:}
		LXNM_NETMASK=${ARG4#*:}
		LXNM_CONNECTED=0
	elif [ "$ARG1" = "RX" ] && [ "${ARG2%:*}" = "packets:" ]; then
		LXNM_RX_PACKETS=${ARG2#*:}
	elif [ "$ARG1" = "TX" ] && [ "${ARG2%:*}" = "packets:" ]; then
		LXNM_TX_PACKETS=${ARG2#*:}
	elif [ "$ARG1" = "RX" ] && [ "$ARG5" = "TX" ]; then
		LXNM_RX_BYTES=${ARG2#*:}
		LXNM_TX_BYTES=${ARG6#*:}
	fi
done <<-EOF
`ifconfig $LXNM_IFNAME`
EOF

while read ARG1 ARG2 ARG3 ARG4 ARG5 ARG6 ARG7; do
	if [ "${ARG4%:*}" = "ESSID" ]; then
		LXNM_ESSID=${ARG4#*:}
		LXNM_ESSID=${LXNM_ESSID//\"}
	elif [ "${ARG1}" = "Link" ]; then
		QUAL=${ARG2#*:}
		QUAL_MAX=${QUAL#*\/}
		QUAL=${QUAL%\/*}
		if [ $QUAL -gt $QUAL_MAX ]; then
			LXNM_QUALITY=100
                elif [ $QUAL -ne 0 ]; then
                        LXNM_QUALITY=$[ QUAL * 100 / QUAL_MAX ]
                fi
	elif [ "${ARG5}" = "Point:" ]; then
		LXNM_BSSID=$ARG6
	fi
done <<-EOF
`iwconfig $LXNM_IFNAME`
EOF

echo -e "$LXNM_ENABLE\n$LXNM_PLUGED\n$LXNM_CONNECTED\n$LXNM_MAC\n$LXNM_IPADDR\n$LXNM_BROADCAST\n$LXNM_NETMASK\n$LXNM_DEST\n$LXNM_RX_BYTES\n$LXNM_RX_PACKETS\n$LXNM_TX_BYTES\n$LXNM_TX_PACKETS\n$LXNM_ESSID\n$LXNM_BSSID\n$LXNM_QUALITY"
