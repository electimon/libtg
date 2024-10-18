AC_INIT(libtg, 1.0, [https://github.com/igkuzm/libtg])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_PROG_CXX

LT_INIT
AC_CHECK_HEADER([sqlite3.h],[],[AC_MSG_ERROR([Please install sqlite3])],[])
AC_CHECK_HEADER([openssl/ssl.h],[],[AC_MSG_ERROR([Please install openssl])],[])

AC_CONFIG_FILES([
Makefile
tl_generator/Makefile
mtx/Makefile
tg/Makefile
])

AC_OUTPUT
