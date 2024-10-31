lib_LTLIBRARIES = libtl.la

libtl_la_SOURCES = serialize.c deserialize.c \
									 deserialize_table.c methods.c tl_send.c
libtl_la_CFLAGS = -fPIC
libtl_la_LIBADD = -lcrypto ../mtx/.libs/libmtx.a
