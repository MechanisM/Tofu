#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([version_major], [0])
m4_define([version_minor], [1])
m4_define([version], [version_major.version_minor])

AC_PREREQ([2.67])
AC_INIT([Tofu],[version],[al3xbio@gmail.com])
AC_CONFIG_SRCDIR([src/tofu.c])

LT_INIT

AM_INIT_AUTOMAKE([Tofu], [version])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

AC_ARG_ENABLE(fcgi, [  --enable-fcgi           build fcgi backend [[default=yes]]],,    enable_fcgi=yes)
AC_ARG_ENABLE(zmq,  [  --enable-zmq            build zmq backend  [[default=yes]]],,    enable_zmq=yes)
AC_ARG_ENABLE(evhttp,  [  --enable-evhttp         build evhttp backend  [[default=yes]]],, enable_evhttp=yes)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h],, AC_MSG_ERROR(stdlib missing.))
AC_CHECK_HEADERS([pcre.h],, AC_MSG_ERROR(pcre missing.))

BACKEND_LDFLAGS=""
BACKEND_DEPS=""

AC_MSG_CHECKING(wheter ZMQ backend should be built)
if test "x$enable_zmq" = "xyes"; then
	AC_MSG_RESULT(yes)
	AC_CHECK_HEADERS([zmq.h],, AC_MSG_ERROR(zmq missing.))
	AC_CHECK_HEADERS([jansson.h],, AC_MSG_ERROR(jansson missing.))
	BACKEND_LDFLAGS="$BACKEND_LDFLAGS -ljansson -lzmq"
	BACKEND_DEPS="$BACKEND_DEPS libtofu_la-zmq.lo"
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(wheter FCGI backend should be built)
if test "x$enable_fcgi" = "xyes"; then
	AC_MSG_RESULT(yes)
	AC_CHECK_HEADERS([fcgi_stdio.h],, AC_MSG_ERROR(fcgi_stdio missing.))
	BACKEND_LDFLAGS="$BACKEND_LDFLAGS -lfcgi"
	BACKEND_DEPS="$BACKEND_DEPS libtofu_la-fcgi.lo"
else
	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(wheter EVHTTP backend should be built)
if test "x$enable_evhttp" = "xyes"; then
	AC_MSG_RESULT(yes)
	AC_CHECK_HEADERS([evhttp.h],, AC_MSG_ERROR(evhttp missing.))
	BACKEND_LDFLAGS="$BACKEND_LDFLAGS -levent"
	BACKEND_DEPS="$BACKEND_DEPS libtofu_la-evhttp.lo"
else
	AC_MSG_RESULT(no)
fi

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([include/config.h])

# Libtool -version-info (ABI version)
LTVER="0:0:0"
AC_SUBST(LTVER)

AC_SUBST(BACKEND_LDFLAGS)
AC_SUBST(BACKEND_DEPS)

AC_CONFIG_FILES([
	Makefile
	eg/Makefile
	src/Makefile
	include/Makefile
])

AC_OUTPUT
