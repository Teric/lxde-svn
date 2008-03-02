#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(lxvt, 0.1, pcman.tw@gmail.com)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/lxvt.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INTLTOOL(, [no-xml])

# Checks for libraries.
pkg_modules="gtk+-2.0 >= 2.6.0 \
             vte"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

GETTEXT_PACKAGE=lxvt
AM_GNU_GETTEXT_VERSION(0.14.1)
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

ALL_LINGUAS="zh_TW"
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    po/Makefile.in
])

AC_OUTPUT
echo
echo lxvt............................. : Version $VERSION
echo
echo Prefix........................... : $prefix
echo
echo Type make and pray!
echo

