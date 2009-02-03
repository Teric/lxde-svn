AC_PREREQ(2.57)
AC_INIT(lxsys-nettool, 0.1, http://lxde.org/)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
AC_PROG_INTLTOOL
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_HEADER_STDC
# Checks for libraries.
pkg_modules="glib-2.0 >= 2.6.0 \
             gtk+-2.0 >= 2.6.0 \
             gthread-2.0"

PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

GETTEXT_PACKAGE=lxsys-nettool
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

ALL_LINGUAS="cs de zh_CN hu fi es et sv it ar"
AM_GLIB_GNU_GETTEXT

AC_OUTPUT([
  Makefile
  src/Makefile
  po/Makefile.in
])

echo
echo LXSys-nettool................................ : Version $VERSION
echo
echo Prefix....................................... : $prefix

