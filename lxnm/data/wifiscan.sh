#!/bin/sh

while read LABEL ARG1 ARG2 ARG3 ARG4 ARG5; do
	if [ A"$LABEL" = A"Cell" ]; then
		PROTO=
		GROUP=
		PAIRWISE=
		KEYMGMT=
		APADDR=$ARG4
	elif [ A"${LABEL%:*}" = A"ESSID" ]; then
		echo -n "+$LXNM_CMDID "
		ESSID=${LABEL#*:}
		ESSID=${ESSID//\"}
		if [ A"$ESSID" = A ]; then
			echo -n "(NULL) "
		else
			echo -n "$ESSID "
		fi

		echo -n "$APADDR "
	elif [ A"${LABEL%:*}" = A"Quality" ]; then
		QUAL=${LABEL#*:}
		QUAL_MAX=${LABEL#*\/}
		QUAL=${QUAL%\/*}
		if [ $QUAL -gt $QUAL_MAX ]; then
			echo -n "100 "
		else
			echo -n "$[ QUAL * 100 / QUAL_MAX ] "
		fi
	elif [ A"${LABEL%=*}" = A"Quality" ]; then
		QUAL=${LABEL#*=}
		QUAL_MAX=${LABEL#*\/}
		QUAL=${QUAL%\/*}
		if [ $QUAL -gt $QUAL_MAX ]; then
			echo -n "100 "
		else
			echo -n "$[ QUAL * 100 / QUAL_MAX ] "
		fi
	elif [ A"$ARG1" = A"key:off" ]; then
		PROTO=OFF
	elif [ A"$ARG1" = A"WPA" ]; then
		PROTO=WPA
	elif [ A"$LABEL" = A"Group" ]; then
		GROUP=$ARG4
	elif [ A"$LABEL" = A"Pairwise" ]; then
		PAIRWISE=$ARG5
	elif [ A"$LABEL" = A"Authentication" ]; then
		KEYMGMT=$ARG5
	elif [ A"$ARG1" = A"Last" ] || [ A"ARG2" = A"beacon" ]; then
		if [ A"$PROTO" = A"OFF" ]; then
			echo OFF
		elif [ A"$PROTO" = A"WPA" ]; then
			echo "$PROTO $KEYMGMT $GROUP $PAIRWISE"
		else
			echo WEP
		fi
	fi
done <<-EOF
$(iwlist wlan0 scan)
EOF
