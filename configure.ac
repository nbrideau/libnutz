AC_INIT([libnutz], [0.0.0.1])
AC_CONFIG_SRCDIR([src/addr.cpp])

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])

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

PKG_CHECK_MODULES(PROTOBUF, protobuf >= 3.0.0)

AC_CHECK_PROG([PROTOC], [protoc], [protoc])
AS_IF([test "x${PROTOC}" == "x"],
    [AC_MSG_ERROR([ProtoBuf compiler "protoc" not found.])])

AC_ARG_WITH([protoc],
    [AS_HELP_STRING([--with-protoc=/path/of/protoc],
        [Location of the protocol buffers compiler protoc. Defaults to looking on path.])],
    [PROTOC="$withval"],
    [ AS_IF([test "x${PROTOC}" == "x"],
        [AC_PATH_PROG([PROTOC], [protoc], [no])])
    ]
)
AS_IF([test "${PROTOC}" == "no"], [AC_MSG_ERROR([ProtoBuf compiler "protoc" not found.])])

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_OUTPUT
