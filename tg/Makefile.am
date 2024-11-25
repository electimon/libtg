lib_LTLIBRARIES = libtg.la
libtg_la_CFLAGS = -fPIC
libtg_la_SOURCES = tg.c \
									 connect.c \
									 database.c \
									 auth.c \
									 strerr.c \
									 messages.c \
									 auth_key.c \
									../crypto/rsa.c \
									../crypto/fact.cpp \
									../crypto/hsh.c \
									../crypto/aes.c \
									../crypto/cmn.c \
									../crypto/rnd.c \
									../crypto/sha1.c \
									../transport/send_query.c \
									../transport/crc.c \
									../transport/net.c

libtg_la_LIBADD = ../tl/*.o -lsqlite3 -lcrypto -lz
libtg_la_LDFLAGS = -Wl,-w
if MACOS
libtg_la_SOURCES += ../transport/darwin-posix-rt/clock_gettime.c
#libtg_la_LIBADD  += ../mtx/darwin-posix-rt/*.o
endif
