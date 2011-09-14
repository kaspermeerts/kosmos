#ifndef KOSMOS_LINEREADER_H
#define KOSMOS_LINEREADER_H

#include <stdio.h>

typedef struct LineReader {
	char *buffer;
	size_t buffer_size;
	size_t line_end;
	FILE *stream;
} LineReader;

LineReader linereader_start(FILE *file);
char *linereader_get(LineReader *lr);
void linereader_stop(LineReader *lr);

#endif
