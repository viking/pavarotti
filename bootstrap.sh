#!/bin/sh

autoreconf --install
./configure
make check
