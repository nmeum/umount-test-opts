#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef min
# define min(x, y) __extension__ ({		\
	__typeof__(x) _min1 = (x);		\
	__typeof__(y) _min2 = (y);		\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
int ul_optstr_next(char **optstr, char **name, size_t *namesz,
		   char **value, size_t *valsz)
{
	int open_quote = 0;
	char *start = NULL, *stop = NULL, *p, *sep = NULL;
	char *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	if (name)
		*name = NULL;
	if (namesz)
		*namesz = 0;
	if (value)
		*value = NULL;
	if (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	while (optstr0 && *optstr0 == ',')
		optstr0++;

	for (p = optstr0; p && *p; p++) {
		if (!start)
			start = p;		/* beginning of the option item */
		if (*p == '"')
			open_quote ^= 1;	/* reverse the status */
		if (open_quote)
			continue;		/* still in quoted block */
		if (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		if (*p == ',')
			stop = p;		/* terminate the option item */
		else if (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		if (!start || !stop)
			continue;
		if (stop <= start)
			return -EINVAL;

		if (name)
			*name = start;
		if (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		if (sep) {
			if (value)
				*value = sep + 1;
			if (valsz)
				*valsz = stop - sep - 1;
		}
		return 0;
	}

	return 1;				/* end of optstr */
}

/*
 * Match string beginning.
 */
static inline const char *startswith(const char *s, const char *prefix)
{
	size_t sz = prefix ? strlen(prefix) : 0;

        if (s && sz && strncmp(s, prefix, sz) == 0)
                return s + sz;
	return NULL;
}

/* caller guarantees n > 0 */
static void xstrncpy(char *dest, const char *src, size_t n)
{
	size_t len = src ? strlen(src) : 0;

	if (!len)
		return;
	len = min(len, n - 1);
	memcpy(dest, src, len);
	dest[len] = 0;
}

/*
 * Option location
 */
struct libmnt_optloc {
	char	*begin;
	char	*end;
	char	*value;
	size_t	valsz;
	size_t  namesz;
};

#define MNT_INIT_OPTLOC	{ .begin = NULL }

#define mnt_optmap_entry_novalue(e) \
		(e && (e)->name && !strchr((e)->name, '=') && !((e)->mask & MNT_PREFIX))

/*
 * Locates the first option that matches @name. The @end is set to the
 * char behind the option (it means ',' or \0).
 *
 * Returns negative number on parse error, 1 when not found and 0 on success.
 */
static int mnt_optstr_locate_option(char *optstr, const char *name,
					struct libmnt_optloc *ol)
{
	char *n;
	size_t namesz, nsz;
	int rc;

	if (!optstr)
		return 1;

	assert(name);

	namesz = strlen(name);
	if (!namesz)
		return 1;

	do {
		rc = ul_optstr_next(&optstr, &n, &nsz,
					&ol->value, &ol->valsz);
		if (rc)
			break;

		if (namesz == nsz && strncmp(n, name, nsz) == 0) {
			ol->begin = n;
			ol->end = *(optstr - 1) == ',' ? optstr - 1 : optstr;
			ol->namesz = nsz;
			return 0;
		}
	} while(1);

	return rc;
}

/**
 * mnt_optstr_get_option:
 * @optstr: string with a comma separated list of options
 * @name: requested option name
 * @value: returns a pointer to the beginning of the value (e.g. name=VALUE) or NULL
 * @valsz: returns size of the value or 0
 *
 * Returns: 0 on success, 1 when not found the @name or negative number in case
 * of error.
 */
int mnt_optstr_get_option(const char *optstr, const char *name,
			  char **value, size_t *valsz)
{
	struct libmnt_optloc ol = MNT_INIT_OPTLOC;
	int rc;

	if (!optstr || !name)
		return -EINVAL;

	rc = mnt_optstr_locate_option((char *) optstr, name, &ol);
	if (!rc) {
		if (value)
			*value = ol.value;
		if (valsz)
			*valsz = ol.valsz;
	}
	return rc;
}

/**
 * mnt_optstr_next_option:
 * @optstr: option string, returns the position of the next option
 * @name: returns the option name
 * @namesz: returns the option name length
 * @value: returns the option value or NULL
 * @valuesz: returns the option value length or zero
 *
 * Parses the first option in @optstr.
 *
 * Returns: 0 on success, 1 at the end of @optstr or negative number in case of
 * error.
 */
static int mnt_optstr_next_option(char **optstr, char **name, size_t *namesz,
					char **value, size_t *valuesz)
{
	if (!optstr || !*optstr)
		return -EINVAL;

	return ul_optstr_next(optstr, name, namesz, value, valuesz);
}

int mnt_match_options(const char *optstr, const char *pattern)
{
	char *name, *pat = (char *) pattern;
	char *buf = NULL, *patval;
	size_t namesz = 0, patvalsz = 0;
	int match = 1;

	if (!pattern && !optstr)
		return 1;
	if (pattern && optstr && !*pattern && !*optstr)
		return 1;
	if (!pattern)
		return 0;

	/* walk on pattern string
	 */
	while (match && !mnt_optstr_next_option(&pat, &name, &namesz,
						&patval, &patvalsz)) {
		char *val;
		size_t sz = 0;
		int no = 0, rc;

		if (*name == '+' && namesz > 1)
			name++, namesz--;
		else if ((no = (startswith(name, "no") != NULL))) {
			name += 2, namesz -= 2;
			if (!*name || *name == ',') {
				match = 0;
				break;	/* alone "no" keyword is error */
			}
		}

		if (optstr && *optstr && *name) {
			if (!buf) {
				buf = malloc(strlen(pattern) + 1);
				if (!buf)
					return 0;
			}

			xstrncpy(buf, name, namesz + 1);
			rc = mnt_optstr_get_option(optstr, buf, &val, &sz);

		} else if (!*name) {
			rc = 0;		/* empty pattern matches */
		} else {
			rc = 1;		/* not found in empty string */
		}

		/* check also value (if the pattern is "foo=value") */
		if (rc == 0 && patvalsz > 0 &&
		    (patvalsz != sz || strncmp(patval, val, sz) != 0))
			rc = 1;

		switch (rc) {
		case 0:		/* found */
			match = no == 0 ? 1 : 0;
			break;
		case 1:		/* not found */
			match = no == 1 ? 1 : 0;
			break;
		default:	/* parse error */
			match = 0;
			break;
		}
	}

	free(buf);
	return match;
}
