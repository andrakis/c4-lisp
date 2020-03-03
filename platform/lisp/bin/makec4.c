/**
 * Make C4
 *
 * A tool to convert portions of a C source file into a .C4 file.
 * The main usage of this tool to is snip contents of a .C file after a given line.
 *
 * Algorithm:
 *    0) Assuming SNIP="// -- SNIP"
 *    1) While $position < ($filelength + strlen($SNIP))
 *    2) If $contents[$position] == $SNIP[0]
 *      2 false) increment $position, Continue loop
 *      2 true) Perform memcmp on position
 *        3 comparefail) increment $position, Continue loop
 *        3 comparesuccess) stop printing and exit
 *
 * See also: combc4, which combines a number of .c4 files into a single .c4 file.
 *
 * Written for C4, but compatible with proper compilers.
 */
// This is a 'native' C4/C5 program.
#include "../../include/core/c5_native.h"

// Streams
enum { STDIN, STDOUT, STDERR };
// Errors
enum {
	E_SUCCESS,        // No error (0)
	E_NOTFOUND,       // The string was not found (still outputs the file)
	E_OPTS,           // Invalid options
	E_FILE,           // Unable to open infile
	E_ALLOC,          // Failed to allocate memory
	E_READ            // Failed to read() the file after opening
};

char *command;
char *infile, *snip, *contents;
int   fsize, returnval;

void usage () {
	dprintf(STDERR, "Usage: %s <infile> <STRING TO SNIP AT>\n", command);
	dprintf(STDERR, "Output will be printed directly to screen\n");
}

int c4_strlen (char *s) { int l; l = 0; while(*s++) ++l; return l; }

void scan_and_snip () {
	int   len;
	char *pos, *endpos, first;

	len = c4_strlen(snip);
	pos = contents;
	endpos = contents + (fsize - len);
	first = *snip;
	while(pos < endpos) {
		if(*pos == first) {
			//dprintf(STDERR, "Got a first match...\n");
			//dprintf(STDERR, "At pos: %.*s\n", len, pos);
			if(!memcmp(pos, snip, len)) {
				returnval = E_SUCCESS;
				printf("%.*s", pos - contents, contents);
				return;
			}
		}
		++pos;
	}
	// The string was never found, output the whole thing
	printf("%s", contents);
	//printf("// The string was never found :(\n");
	returnval = E_NOTFOUND;
}

int main (int argc, char **argv) {
	int fd, bytesread;

	command = *argv;
	++argv; --argc;
	if(argc < 2) {
		usage();
		return E_OPTS;
	}

	infile = *argv; ++argv; --argc;
	snip = *argv; ++argv; --argc;

	if((fd = open(infile, 0)) < 0) {
		dprintf(STDERR, "Failed to open file %s\n", infile);
		close(fd);
		return E_FILE;
	}
	fsize = fdsize(fd) + 1;

	if(!(contents = malloc(fsize))) {
		dprintf(STDERR, "Failed to allocate %ld bytes for contents\n", fsize);
		close(fd);
		return E_ALLOC;
	}
	bytesread = read(fd, contents, fsize);
	close(fd); // Finished with file
	if(bytesread <= 0) {
		dprintf(STDERR, "Failed to read %ld bytes from file after opening\n", fsize);
		close(fd);
		free(contents);
		return E_READ;
	}

	scan_and_snip();

	// Cleanup
	free(contents);
	return returnval;
}
