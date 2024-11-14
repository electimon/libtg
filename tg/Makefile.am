lib_LTLIBRARIES = libtg.la
libtg_la_CFLAGS = -fPIC
libtg_la_SOURCES = tg.c connect.c database.c auth.c \
									 strerr.c
libtg_la_LIBADD = ../mtx/src/*.o ../tl/*.o -lsqlite3 -lcrypto
libtg_la_LDFLAGS = -Wl,-w
if MACOS
libtg_la_LIBADD += ../mtx/darwin-posix-rt/*.o
endif


