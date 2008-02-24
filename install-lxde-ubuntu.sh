#!/bin/sh

echo 'Install packages needed for building LXDE from source code.'

apt-get -y install subversion libgtk2.0-dev libgamin-dev pkg-config libstartup-notification0-dev shared-mime-info desktop-file-utils libhal-dev libdbus-glib-1-dev libhal-storage-dev automake1.9 autoconf libxmu-dev libxpm-dev libsm-dev libice-dev smproxy libjpeg62-dev autotools-dev build-essential

mkdir lxde_build
cd lxde_build

echo 'Downloading the latest source code of LXDE from svn.'

svn co https://lxde.svn.sourceforge.net/svnroot/lxde/trunk lxde

svn co https://pcmanfm.svn.sourceforge.net/svnroot/pcmanfm/trunk pcmanfm

echo 'Compiling LXDE...'

cd lxde
./install-lxde.sh "$@"
cd ..

cd pcmanfm
./configure "$@"
make install clean

echo "LXDE is installed."
echo
echo "Now you can login to LXDE in GDM."
echo
echo "Have fun!"

