#ifndef KOSMOS_UTIL
#define KOSMOS_UTIL

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(x) #x

const char *path_filename(const char *name);

#endif
