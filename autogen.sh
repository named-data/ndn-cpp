#!/bin/sh -e

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.
autoreconf --force --install --verbose "$srcdir"
# Don't run ./configure since the user may want to run ./configure with options.
echo "" ; echo "You need to run ./configure again."
