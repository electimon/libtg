lib_LTLIBRARIES = libtg.la
libtg_la_CFLAGS = -fPIC
libtg_la_SOURCES = tg.c tg_connect.c database.c 
libtg_la_LIBADD = -lcrypto ../tl/*.o -lsqlite3
libtg_la_LDFLAGS = -Wl,-w
