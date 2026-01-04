#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "busybox.h"
#include "util-linux.h"

#include <klee/klee.h>

/* locale-indepdent implementation of isalpha from musl libc */
static int isalpha(int c)
{
	return ((unsigned)c|32)-'a' < 26;
}

int
main(int argc, char **argv)
{
	char options[INPUT_SIZE];
	char pattern[INPUT_SIZE];

	klee_make_symbolic(&options, sizeof(options), "options");
	for (int i = 0; i < sizeof(options) - 1; i++)
		klee_assume(isalpha(options[i]) | (options[i] == ',') | (options[i] == '+'));
	klee_assume(options[sizeof(options) - 1] == '\0');

	klee_make_symbolic(&pattern, sizeof(pattern), "pattern");
	for (int i = 0; i < sizeof(pattern) - 1; i++)
		klee_assume(isalpha(pattern[i]) | (pattern[i] == ',') | (pattern[i] == '+'));
	klee_assume(pattern[sizeof(pattern) - 1] == '\0');

#if 0
	printf("options: %s\n", options);
	printf("pattern: %s\n", pattern);
#endif

	assert(mnt_match_options(options, pattern) == fsopts_matches(options, pattern));
	return 0;
}
