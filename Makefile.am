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
	src/utils/http.h	\
	src/utils/common.h	\
	src/bstring/bstrlib.h

libtofu_la_SOURCES =			\
	src/bstring/bstrlib.c	\
	src/ctx.c		\
	src/handler.c		\
	src/rep.c		\
	src/req.c		\
	src/tofu.c

if FCGI
	libtofu_la_SOURCES = $(libtofu_la_SOURCES) src/backend/fcgi.c
endif

if ZMQ
	libtofu_la_SOURCES = $(libtofu_la_SOURCES) src/backend/zmq.c
endif

if EVHTTP
	libtofu_la_SOURCES = $(libtofu_la_SOURCES) src/backend/evhttp.c
endif

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/tofu

libtofu_la_CFLAGS = -Wall -O3 -fomit-frame-pointer
libtofu_la_LDFLAGS = -version-info @LTVER@  -lpcre @AM_LDFLAGS@

#bin_PROGRAMS =			\
#	example_evhttp		\
#	example_fcgi		\
#	example_zmq		\
#	example_errors		\
#	example_status		\
#	example_hi

#LDADD = libtofu.la

#example_evhttp_SOURCES = eg/example-evhttp.c
#example_fcgi_SOURCES = eg/example-fcgi.c
#example_zmq_SOURCES = eg/example-zmq.c
#example_errors_SOURCES = eg/example-errors.c
#example_status_SOURCES = eg/example-status.c
#example_hi_SOURCES = eg/example-hi.c
