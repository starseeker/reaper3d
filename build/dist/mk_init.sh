#!/bin/sh

VERSION=${VERSION:-snapshot}

DEFSYS=`uname`

SYS=${SYS:-$DEFSYS}

case $SYS in
 ?inux)
    HOST=linux
    VER=linux
    TG=gcc3
    MKFLAGS1="dlnk=-static-libgcc CXX=gcc-3.0"
    MKFLAGS2="lnk=-static-libgcc -rdynamic -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic"
    ;;
 CYG*)
    HOST=win32
    VER=win32
    TG=win32
    EXT=.exe
    AEXT=.exe
    ;;
 cross)
    HOST=linux
    VER=win32
    TG=xgcc
    AEXT=.exe
esac


TMP=${TMP:-/tmp}

DIR=${TMP}/reaper-${VERSION}-clean
DEST=reaper-${VERSION}

PATH=$PATH:`pwd`

export TMP
export PATH
export DIR
export DEST
export HOST
export VER
export TG
export EXT
export AEXT
