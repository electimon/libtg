lib_LTLIBRARIES = libtl.la

libtl_la_SOURCES = serialize.c deserialize.c \
									 deserialize_table.c methods.c \
									 tl_send.c tl.c gunzip.c
libtl_la_CFLAGS = -fPIC
libtl_la_LIBADD = -lcrypto ../mtx/src/*.o
libtl_la_LDFLAGS = -Wl,-w
