AC_INIT([libtg],[1.1],[https://github.com/igkuzm/libtg])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CXX

AC_ARG_WITH([tgtest],
						[AS_HELP_STRING([--with-tgtest],
														[enable tgtest (default=no)])],
						[enable_tgtest=$withval],
						[enable_tgtest=no])
AM_CONDITIONAL([MAKE_TEST], [test "$enable_tgtest" = "yes"])

AC_CANONICAL_HOST
case "${host_os}" in
    cygwin*|mingw*)
        build_windows=yes;;
    darwin*)
        build_mac=yes;;
    *)
		build_linux=yes;;
esac

case "${host_cpu}" in
    i?86*)
        build_i386=yes;;
    x86_64*)
        build_x86_64=yes;;
    arm64*|aarch64*)
        build_arm64=yes;;
    armv7*)
        build_arm=yes;;
    *)
		;;
esac

AM_CONDITIONAL([LINUX],   [test "$build_linux"   = "yes"])
AM_CONDITIONAL([MACOS],   [test "$build_mac"     = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([WIN32],   [test "$build_windows" = "yes" -a "$build_i386" == "yes"])
AM_CONDITIONAL([WIN64],   [test "$build_windows" = "yes" -a "$build_x86_64" == "yes"])

LT_INIT
AC_CHECK_HEADER([sqlite3.h],[],[AC_MSG_ERROR([Please install sqlite3])],[])
AC_CHECK_HEADER([openssl/ssl.h],[],[AC_MSG_ERROR([Please install openssl])],[])
#AC_CHECK_HEADER([opus/opus.h],[],[AC_MSG_ERROR([Please install opus])],[])
#AC_CHECK_HEADER([opus/opusenc.h],[],[AC_MSG_ERROR([Please install opusenc])],[])
#AC_CHECK_HEADER([opus/opusfile.h],[],[AC_MSG_ERROR([Please install opusfile])],[])

AC_CONFIG_FILES([
Makefile
tl_generator/Makefile
mtx/Makefile
tl/Makefile
tg/Makefile
])

AC_OUTPUT
