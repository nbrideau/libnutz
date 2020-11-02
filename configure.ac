AC_INIT([libnutz], [0.0.0.1])
AC_CONFIG_SRCDIR([src/sockbase.cpp])

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AM_PROG_LIBTOOL

CFLAGS=" -D_REENTRANT -pthread -O2 -pipe $CFLAGS "
AC_ARG_ENABLE(debug,
[  --enable-debug          debugging mode],
if test "$enable_debug" = "yes"; then
    CFLAGS=" -D_REENTRANT -pthread -ggdb -pipe -Wall "
fi)
CXXFLAGS=" $CFLAGS "

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_OUTPUT
