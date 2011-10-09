#include <stdarg.h>
#include <stdio.h>

#include "log.h"

static void vlog_msg(const char *fmt, va_list arg)
{
	/* TODO: Add more loggers, like log to file */
	vfprintf(stderr, fmt, arg);
}

void log_err(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vlog_msg(format, args);
	va_end(args);
}

void log_dbg(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vlog_msg(format, args);
	va_end(args);
}
