#include <string.h>

static int fsopt_matches(const char *opts_list, const char *opt, size_t optlen)
{
	int match = 1;

	if (optlen >= 2 && opt[0] == 'n' && opt[1] == 'o') {
		match--;
		opt += 2; optlen -= 2;
	}

	if (optlen == 0)
		return 0;
	if (match && optlen > 1 && *opt == '+') {
		opt++; optlen--;
	}

	while (1) {
		if (strncmp(opts_list, opt, optlen) == 0) {
			const char *after_opt = opts_list + optlen;
			if (*after_opt == '\0' || *after_opt == ',')
				return match;
		}

		opts_list = strchr(opts_list, ',');
		if (!opts_list)
			break;
		opts_list++;
	}

	return !match;
}

int fsopts_matches(const char *opts_list, const char *reqopts_list)
{
	if (!reqopts_list)
		return 1; /* no options requested, match anything */

	while (1) {
		size_t len;
		const char *comma = strchr(reqopts_list, ',');
		if (!comma)
			len = strlen(reqopts_list);
		else
			len = comma - reqopts_list;

		if (len && !fsopt_matches(opts_list, reqopts_list, len))
			return 0;

		if (!comma)
			break;
		reqopts_list = ++comma;
	}

	return 1;
}
