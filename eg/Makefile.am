INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/tofu
LDADD    = $(top_srcdir)/src/libtofu.la

bin_PROGRAMS =		\
	example_evhttp	\
	example_fcgi	\
	example_zmq	\
	example_errors	\
	example_status	\
	example_hi	\
	example_post

example_evhttp_SOURCES = example-evhttp.c
example_fcgi_SOURCES   = example-fcgi.c
example_zmq_SOURCES    = example-zmq.c
example_errors_SOURCES = example-errors.c
example_status_SOURCES = example-status.c
example_hi_SOURCES     = example-hi.c
example_post_SOURCES   = example-post.c
