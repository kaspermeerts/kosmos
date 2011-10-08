#ifndef KOSMOS_UTIL
#define KOSMOS_UTIL

#include <stdio.h>

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(x) #x

const char *path_filename(const char *name);
long fsize(FILE *stream);

#endif
