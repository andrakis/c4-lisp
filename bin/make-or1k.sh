#!/bin/bash

set -e

prefix=or1k-linux-musl
cc=$prefix-gcc
cxx=$prefix-g++
files="bin lib/ lisp4.c c4.c c5.c c4_modules"
platform="or1k-musl"
bin_debug="dist/Debug/$platform/lisp4"
bin_release="dist/Release/$platform/lisp4"
pkg_dir="or1k"
pkg_tar_opts="cz"
pkg_file="c4-or1k.tgz"

PATH=/opt/$prefix/bin:$PATH

tries=0

shopt -s nocasematch
if [[ "$1" =~ ^rel ]]; then
	conf=Release
	out=release
else
	conf=Debug
	out=debug
fi

build() {
	make CONF="$conf" CC="$cc" CXX="$cxx" -j2 CND_PLATFORM=$platform || clean_and_build
}

clean() {
	make CONF="$conf" clean
}

clean_and_build() {
	(( tries += 1 ))
	if [ "$tries" -lt 2 ]; then
		clean && build
	else
		echo "Error in compilation"
		exit 2
	fi
}

run_steps() {
	build
	mkdir -p $pkg_dir
	cd $pkg_dir
	mkdir -p bin lib
	cp ../$bin_debug bin/debug || true
	cp ../$bin_release bin/release || true
	cp -L ../*.c .
	cp ../lib/*-or1k-musl.so lib
	cp -r ../c4_modules .
	tar -$pkg_tar_opts -f ../$pkg_file $files
	tar ttf ../$pkg_file
	cd ..
	echo "$pkg_file created."
	ls -lh $pkg_file
	ls -l  $pkg_file
}

run_steps
