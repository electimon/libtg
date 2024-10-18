bin_PROGRAMS = generator
generator_SOURCES = generator.c tl_parser.c tl_parser.h telegram-api.tl 

generator_CFLAGS = 
generator_LDADD = 

../tg/methods.c: generator
	./generator

all-local: ../tg/methods.c 
