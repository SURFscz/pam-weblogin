#!/bin/sh

aclocal \
&& automake --warnings none --add-missing \
&& autoconf
