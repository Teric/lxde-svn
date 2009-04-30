#!/bin/sh

while read LABEL ARG1 ARG2 ARG3 ARG4 ARG5; do
	if [ A"$LABEL" = A"Cell" ]; then
		RSLINE=
		PROTO=
		GROUP=
		PAIRWISE=
		KEYMGMT=
		APADDR=$ARG4
	elif [ A"${LABEL%:*}" = A"ESSID" ]; then
		RSLINE="+$LXNM_CMDID "
		ESSID=${LABEL#*:}
		ESSID=${ESSID//\"}
		if [ A"$ESSID" = A ]; then
			RSLINE=$RSLINE"(NULL) "
		else
			RSLINE=$RSLINE"$ESSID "
		fi

		RSLINE=$RSLINE"$APADDR "
	elif [ A"${LABEL%:*}" = A"Quality" ]; then
		QUAL=${LABEL#*:}
		QUAL_MAX=${LABEL#*\/}
		QUAL=${QUAL%\/*}
		if [ $QUAL -gt $QUAL_MAX ]; then
			RSLINE=$RSLINE"100 "
		else
			RSLINE=$RSLINE"$[ QUAL * 100 / QUAL_MAX ] "
		fi
	elif [ A"${LABEL%=*}" = A"Quality" ]; then
		QUAL=${LABEL#*=}
		QUAL_MAX=${LABEL#*\/}
		QUAL=${QUAL%\/*}
		if [ $QUAL -gt $QUAL_MAX ]; then
			RSLINE=$RSLINE"100 "
		else
			RSLINE=$RSLINE"$[ QUAL * 100 / QUAL_MAX ] "
		fi
	elif [ A"$ARG1" = A"key:off" ]; then
		PROTO=OFF
	elif [ A"$ARG1" = A"WPA" ]; then
		PROTO=WPA
	elif [ A"$LABEL" = A"Group" ]; then
		GROUP=$ARG3
	elif [ A"$LABEL" = A"Pairwise" ]; then
		PAIRWISE=$ARG4
	elif [ A"$LABEL" = A"Authentication" ]; then
		KEYMGMT=$ARG4
	elif [ A"$ARG1" = A"Last" ] && [ A"$ARG2" = A"beacon:" ]; then
		if [ A"$PROTO" = A"OFF" ]; then
			RSLINE=$RSLINE"OFF"
		elif [ A"$PROTO" = A"WPA" ]; then
			RSLINE=$RSLINE"$PROTO $KEYMGMT $GROUP $PAIRWISE"
		else
			RSLINE=$RSLINE"WEP"
		fi

		echo $RSLINE
	fi
done <<-EOF
`iwlist $LXNM_IFNAME scan`
EOF
