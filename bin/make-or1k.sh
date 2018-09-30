#!/bin/bash

set -e

prefix=or1k-linux-musl
cc=$prefix-gcc
cxx=$prefix-g++
files="bin/debug bin/release rtl/GNU-Linux/* lisp4.c c4.c c5.c"

PATH=/opt/$prefix/bin:$PATH

shopt -s nocasematch
if [[ "$1" =~ ^rel ]]; then
	conf=Release
	out=release
else
	conf=Debug
	out=debug
fi

build() {
	make CONF="$conf" CC="$cc" CXX="$cxx" -j2 || clean_and_build
}

clean() {
	make CONF="$conf" clean
}

clean_and_build() {
	clean
	build
}

run_steps() {
	build
	ls -lhH bin/$out rtl/GNU-Linux/*
	set -x
	tar chvf c4-or1k.tar $files
	set +x
}

run_steps
