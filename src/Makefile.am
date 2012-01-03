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

libtofu_la_CFLAGS = -Wall -O3
libtofu_la_LDFLAGS = -export-symbols-regex '^tofu_'	\
	-version-info 0:0:0 				\
	@BACKEND_LDFLAGS@

libtofu_la_LIBADD = @BACKEND_DEPS@ -lpcre
libtofu_la_DEPENDENCIES = @BACKEND_DEPS@
