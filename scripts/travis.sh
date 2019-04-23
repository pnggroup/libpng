#!/bin/bash

set -e

if [[ $ENABLE_SANITIZERS ]]; then
    LOCAL_CMAKE_EXTRA_ARGS="-DCMAKE_C_FLAGS=-fsanitize=$ENABLE_SANITIZERS"
    LOCAL_MAKE_EXTRA_ARGS="CFLAGS=-fsanitize=$ENABLE_SANITIZERS LDFLAGS=-fsanitize=$ENABLE_SANITIZERS"
fi

if [[ $USE_CMAKE ]]; then
    echo "$0: using cmake + make:" $LOCAL_CMAKE_EXTRA_ARGS $EXTRA_ARGS
    mkdir build-cmake
    cd build-cmake
    cmake $LOCAL_CMAKE_EXTRA_ARGS $EXTRA_ARGS ..
    make
    [[ $DISABLE_TESTS ]] || make test
    make clean
fi

if [[ $USE_CONFIGURE ]]; then
    mkdir build-configure
    cd build-configure
    echo "$0: using configure + make:" $LOCAL_MAKE_EXTRA_ARGS $EXTRA_ARGS
    ../configure $LOCAL_MAKE_EXTRA_ARGS $EXTRA_ARGS
    make
    [[ $DISABLE_TESTS ]] || make test
    make clean
    make distclean
fi

if [[ $USE_LEGACY_MAKEFILES ]]; then
    echo "$0: using scripts/makefile.$CC:" $LOCAL_MAKE_EXTRA_ARGS $EXTRA_ARGS
    make -f scripts/makefile.$CC $LOCAL_MAKE_EXTRA_ARGS $EXTRA_ARGS
    [[ $DISABLE_TESTS ]] || make -f scripts/makefile.$CC $LOCAL_MAKE_EXTRA_ARGS $EXTRA_ARGS test
    make -f scripts/makefile.$CC $LOCAL_MAKE_EXTRA_ARGS $EXTRA_ARGS clean
    # TODO: use scripts/makefile.std, etc.
fi
