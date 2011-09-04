#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linereader.h"

const size_t MAX_BUFFER_LEN = 80; /* ? */

LineReader linereader_start(FILE *stream)
{
	LineReader lr;

	lr.stream = stream;
	lr.buffer = malloc(MAX_BUFFER_LEN);
	lr.buffer_size = 0;
	lr.line_end = 0;

	return lr;
}

char *linereader_get(LineReader *lr)
{
	size_t num_read;
	size_t buffer_pos = 0;

	/* Remove the last line from the buffer */
	if (lr->line_end != 0)
	{
		memmove(lr->buffer, lr->buffer + lr->line_end, 
				lr->buffer_size - lr->line_end);
		lr->buffer_size -= lr->line_end;
		lr->line_end = 0;
	}

	do
	{
		/* Fetch more if necessary */
		if (buffer_pos >= lr->buffer_size)
		{
			if (lr->buffer_size >= MAX_BUFFER_LEN)
				return NULL; /* Out of memory, realloc? XXX */

			num_read = fread(lr->buffer + lr->buffer_size, 1, 
					MAX_BUFFER_LEN - lr->buffer_size, lr->stream);
			if (num_read == 0)
				return NULL; /* FIXME EOF or error */
			lr->buffer_size += num_read;
		}

		while (buffer_pos < lr->buffer_size && lr->buffer[buffer_pos] != '\n')
			buffer_pos++;
		
	} while (buffer_pos >= lr->buffer_size || lr->buffer[buffer_pos] != '\n');

	lr->buffer[buffer_pos] = '\0';
	lr->line_end = buffer_pos + 1;

	return lr->buffer;
}

void linereader_stop(LineReader *lr)
{
	free(lr->buffer);
}
