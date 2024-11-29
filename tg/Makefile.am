lib_LTLIBRARIES = libtg.la
libtg_la_CFLAGS = -fPIC
libtg_la_SOURCES = tg.c \
				   connect.c \
				   database.c \
				   auth.c \
				   auth_key.c \
				   strerr.c \
				   dialogs.c \
           ../crypto/hsh.c \
           ../crypto/aes.c \
           ../crypto/cmn.c \
           ../crypto/rsa.c \
				   ../transport/send_query.c \
				   ../transport/encrypt.c \
				   ../transport/header.c \
				   ../transport/transport.c \
				   ../transport/crc.c \
				   ../transport/net.c

libtg_la_LIBADD = ../tl/*.o ../mtx/src/*.o -lsqlite3 -lcrypto -lz
libtg_la_LDFLAGS = -Wl,-w
if MACOS
libtg_la_SOURCES += ../transport/darwin-posix-rt/clock_gettime.c
#libtg_la_LIBADD  += ../mtx/darwin-posix-rt/*.o
endif
