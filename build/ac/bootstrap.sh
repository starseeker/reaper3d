#!/bin/sh

case `automake --version` in
 *\ 1.5*) ;;
 *\ 1.6*) ;;
 *)    echo "automake version 1.5+ required!"
       exit 1
esac
 
./gen_source_defs.pl

aclocal && \
autoheader && \
automake --add-missing --copy --foreign && \
autoconf

