AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libtofu.la

include_HEADERS =		\
	include/tofu/ctx.h	\
	include/tofu/rep.h	\
	include/tofu/req.h	\
	include/tofu/handler.h


HEADERS_noinst =		\
	src/backend.h		\
	src/utils/list.h	\
	src/utils/common.h	\
	src/bstring/bstraux.h	\
	src/bstring/bstrlib.h

libtofu_la_SOURCES =		\
	src/bstring/bstraux.c	\
	src/bstring/bstrlib.c	\
	src/ctx.c		\
	src/handler.c		\
	src/rep.c		\
	src/req.c		\
	src/tofu.c		\
	src/backend/fcgi.c

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/tofu

libtofu_la_CFLAGS = -Wall -O3 -fomit-frame-pointer
libtofu_la_LDFLAGS = -version-info @LTVER@  -lpcre -lfcgi -lzmq -ljansson

bin_PROGRAMS = tofu_example
tofu_example_LDADD = libtofu.la
tofu_example_SOURCES = eg/example.c
