#!/bin/sh

SUBDIRS="lxsession lxpanel gpicview lxde-data"

for sub in $SUBDIRS
do
    cd "$sub"

    echo
    echo "Configuring $sub"
    echo

    ./configure "$@"
    cd ..
done

for sub in $SUBDIRS
do

    echo
    echo "Installing $sub"
    echo

    cd "$sub"
    make install-strip clean
    cd ..
done

