INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/tofu

lib_LTLIBRARIES = libtofu.la

libtofu_la_SOURCES =		\
	bstring/bstrlib.c	\
	ctx.c handler.c rep.c	\
	req.c tofu.c

EXTRA_libtofu_la_SOURCES = 	\
	backend/fcgi.c		\
	backend/zmq.c		\
	backend/evhttp.c

libtofu_la_CFLAGS = -Wall -O3 -fomit-frame-pointer
libtofu_la_LDFLAGS = -version-info @LTVER@  -lpcre @BACKEND_LDFLAGS@
libtofu_la_LIBADD = @BACKEND_DEPS@
libtofu_la_DEPENDENCIES = @BACKEND_DEPS@
