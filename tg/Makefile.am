lib_LTLIBRARIES = libtg.la
libtg_la_CFLAGS = -fPIC
libtg_la_SOURCES = tg.c connect.c database.c auth.c \
									 strerr.c messages.c auth_key.c \
									../crypto/cry.c ../crypto/fact.cpp ../crypto/hsh.c \
									../crypto/cmn.c \
									../transport/send_query.c ../transport/net.c
libtg_la_LIBADD = ../tl/*.o -lsqlite3 -lcrypto
libtg_la_LDFLAGS = -Wl,-w
if MACOS
libtg_la_LIBADD += ../mtx/darwin-posix-rt/*.o
endif


