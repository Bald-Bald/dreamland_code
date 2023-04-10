#!/bin/bash

ROOT=/home/travis/build/dreamland-mud
RUNTIME=$ROOT/runtime
SRC=$TRAVIS_BUILD_DIR

run_build() {
    make -f Makefile.git && \
    mkdir -p $RUNTIME && \
    cd $RUNTIME && \
    $SRC/configure --prefix=$RUNTIME --disable-dependency-tracking && \
    make -j 2 && make install
}

run_smoke_test() {
    cd $RUNTIME && \
    git clone https://github.com/dreamland-mud/dreamland_world.git share/DL && \
    ./bin/dreamland $SRC/admin/travis/dreamland.xml 
}

travis_script() {
    run_build && run_smoke_test
}

set -x # display expanded commands

$1;

