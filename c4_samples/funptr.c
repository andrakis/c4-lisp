/**
 * C4 Sample: Function Pointer
 *
 * Tests the behaviour of calling function pointers in C4.
 * Result: not implemented in C4
 */

int f_add (int a, int b) { return a + b; }
int f_sub (int a, int b) { return a - b; }

int main (int argc, char **argv) {
	int *f, x;

	f = f_add;
	x = f(2, 3);
	printf("Result: %ld\n", x);

	return 0;
}
