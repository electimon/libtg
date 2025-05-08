#ifndef HAVE_STRNDUP
#define HAVE_STRNDUP
char *_strndup(const char *s, size_t n);
#define strndup _strndup
#endif /* HAVE_STRNDUP */
