#include <stdio.h>

#include "util.h"

#ifdef _WIN32
#define IS_SEPARATOR(ch) ((ch) == '/' || (ch) == '\\')
#else
#define IS_SEPARATOR(ch) ((ch) == '/')
#endif

const char *path_filename(const char *name)
{
	const char *base;

	for (base = name; *name != '\0'; name++)
	{
		if (IS_SEPARATOR(*name))
			base = name + 1;
	}

	return base;
}

long fsize(FILE *stream)
{
	long cur_off, size;

	cur_off = ftell(stream);
	if (cur_off == -1)
		return -1;

	if (fseek(stream, 0, SEEK_END) == -1)
		return -1;

	if ((size = ftell(stream)) == -1)
		return -1;

	fseek(stream, cur_off, SEEK_SET);

	return size;
}
