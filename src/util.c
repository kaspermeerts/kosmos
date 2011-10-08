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
