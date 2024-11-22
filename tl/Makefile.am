lib_LTLIBRARIES = libtl.la

libtl_la_SOURCES = serialize.c deserialize.c \
									 deserialize_table.c methods.c \
									 send_query.c tl.c gunzip.c \
									 buf.c net.c hsh.c sha1.c sha256.c \
									 cry.c cmn.c rsa.c
libtl_la_CFLAGS = -fPIC
libtl_la_LIBADD = -lcrypto ../mtx/src/*.o
libtl_la_LDFLAGS = -Wl,-w
