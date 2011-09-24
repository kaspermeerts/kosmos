#ifndef KOSMOS_LOG
#define KOSMOS_LOG

void log_err(const char *format, ...) __attribute__((format(printf,1,2)));
void log_dbg(const char *format, ...) __attribute__((format(printf,1,2)));

#endif
