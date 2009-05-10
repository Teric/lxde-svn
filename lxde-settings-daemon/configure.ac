AC_PREREQ([2.57])
AC_INIT(lxde-settings-daemon,[0.4.2], [],lxde-settings-daemon)
AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip])
dnl AC_PROG_INTLTOOL(, [no-xml])
dnl AM_MAINTAINER_MODE

AC_CONFIG_SRCDIR([src/lxde-settings-daemon.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_LN_S
AM_PROG_CC_C_O

# Checks for header files.
AC_PATH_X
AC_STDC_HEADERS
AC_CHECK_HEADERS([locale.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([setlocale])

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.6.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

AC_OUTPUT([
  Makefile
  src/Makefile
  man/Makefile
])

