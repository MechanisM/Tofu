Tofu
====

![status](http://stillmaintained.com/AlexBio/Tofu.png)

**Tofu** is a framework for building web application in C with as minimal effort
as possible, allowing simple web applications to be created rapidly and with few
lines of C code.

Tofu is web server agnostic, which means that applications written using the Tofu
API can be run on a wide variety of backend web servers with very little work on
the app developer side.

**Attention:** Tofu is currently in early development stage, which means that it
is not really suitable for production code (not that the project is intended to
be used in production anyway) and that its API may change in the next future.

## C? WTF?

C is a really fun language to develop with once you get used to it. Software
written in C is also (most of the times) pretty fast and with low memory footprint
which makes the language ideal for performance-critical environments.

## GETTING STARTED

Put the following code into `hi.c`:

~~~~ c
#include <tofu.h>

tofu_rep_t *hello(tofu_req_t *req) {
  tofu_rep_t *rep = tofu_rep_init();

  tofu_head(rep, "Content-Type", "text/html");
  tofu_write(rep, "Hello World!");

  return rep;
}

void main() {
  char *opts[] = { "0.0.0.0", "8080" };
  tofu_ctx_t *ctx = tofu_ctx_init(TOFU_BACKEND_EVHTTP, opts);

  tofu_handle_with(ctx, GET, "/hi", hello);
  tofu_loop(ctx);
}
~~~~

compile and run with:

~~~~
$ cc -o hi hi.c -ltofu
$ ./hi
~~~~

then visit [http://0.0.0.0:8080/hi](http://0.0.0.0:8080/hi).

## OVERVIEW

A Tofu web application consists in one or more route handlers, which, basically,
are functions that take as argument a Tofu request (`tofu_req_t`) and return a
Tofu response (`tofu_rep_t`). Every route handler has to be registered and
associated with an HTTP method (`GET` `POST` ...) and a route pattern, with the
`tofu_handle_with()` function:

~~~~ c
tofu_handle_with(ctx, GET, "/route/:param", handler_function);
~~~~

A route pattern can contain one or more tokens (words prefixed by `:`). Each token
found in a route pattern is then included in the Tofu request so that it can be
accessed by the route handler. For example the route `/page/:name` is matched
against the `/page/index` URI, and the corrispondent handler can fetch the `name`
paramenter with the `tofu_param()` function:

~~~~ c
char *param = tofu_param(req, "name");
~~~~

An handler can be also defined to only handle execution errors, for example `404`
or `500` errors. To register an error handler the function `tofu_rescue_with()`
has to be used:

~~~~ c
tofu_rescue_with(ctx, 404, err404_handler_func);
~~~~

By default Tofu can handle some errors (like 404s or 500s) if no other handler
has been defined in the application.

During the initialization phase the application has also to define and initialize
a Tofu context (`tofu_ctx_t`) which holds all the information needed to run the
application. The app backend can be chosen in this phase.

## DEPENDENCIES

 * `pcre`
 * `libfcgi` (only for the fcgi backend)
 * `libevent` (>= 2.0) (only for the evhttp backend)
 * `jansson` (only for the zmq backend)
 * `zeromq` (only for the zmq backend)

## BUILDING

Tofu is distributed as source code. Build with:

~~~~
$ git clone git://github.com/AlexBio/Tofu.git && cd Tofu
$ ./autogen.sh
$ ./configure
$ make
~~~~

By default all the backends are built.

## COPYRIGHT

Copyright (C) 2011 Alessandro Ghedini <al3xbio@gmail.com>

See COPYING for the license.
