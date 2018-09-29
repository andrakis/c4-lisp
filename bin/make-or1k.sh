#!/bin/bash

set -e

prefix=or1k-linux-musl
cc=$prefix-gcc
cxx=$prefix-g++

PATH=/opt/$prefix/bin:$PATH

shopt -s nocasematch
if [[ "$1" =~ ^rel ]]; then
	conf=Release
	out=release
else
	conf=Debug
	out=debug
fi

#make CONF="$conf" clean
make CONF="$conf" CC="$cc" CXX="$cxx"
ls -lhH bin/$out rtl/GNU-Linux/*
