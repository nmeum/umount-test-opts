#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "busybox.h"
#include "util-linux.h"

#include <klee/klee.h>

int
main(int argc, char **argv)
{
	char options[128];
	char pattern[128];

	klee_make_symbolic(&options, sizeof(options), "options");
	for (int i = 0; i < sizeof(options); i++)
		klee_assume(isalpha(options[i]) || options[i] == ',');
	klee_assume(options[sizeof(options) - 1] == '\0');

	klee_make_symbolic(&pattern, sizeof(pattern), "pattern");
	for (int i = 0; i < sizeof(pattern); i++)
		klee_assume(isalpha(pattern[i]) || pattern[i] == ',');
	klee_assume(options[sizeof(pattern) - 1] == '\0');

#if 0
	printf("options: %s\n", options);
	printf("pattern: %s\n", pattern);
#endif

	assert(mnt_match_options(options, pattern) == fsopts_matches(options, pattern));
	return 0;
}
