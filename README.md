Tofu
====

![status](http://stillmaintained.com/AlexBio/Tofu.png)

Tofu is an open-source web-framework, that lets programmers write their web
application independently from the underlaying web server.

It is written in C so that it can be used from wathever language you want,
provided that the proper binding exists.

The Tofu's architecture makes possible to write applications that can run on
many different web servers (e.g. nginx, Lighttpd, Apache, Mongrel2, ...) without
the need to completely re-write the app code to support them.

It is still in early development, but it can be already used to write simple
web applications.

## INSTALLATION

Tofu is distributed as source code. Install with:

    $ git clone git://github.com/AlexBio/Tofu.git && cd Tofu
    $ ./autogen.sh
    $ ./configure
    $ make

## COPYRIGHT

Copyright (C) 2011 Alessandro Ghedini <al3xbio@gmail.com>

See COPYING for the license.
