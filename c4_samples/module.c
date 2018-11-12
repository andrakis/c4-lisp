// C4 module test
#include <stdio.h>

// Non-C4 stuff
#define module(Name)

module("c4_samples/module_mod.c");

int main (int argc, char **argv) {
	int x, y;

	x = 4; y = 5;

	// call module function 'add'
	printf("%ld + %ld = %ld\n", x, y, add(x, y));
}
