#ifndef KOSMOS_STATS
#define KOSMOS_STATS

#include "font.h"
#include "shader.h"

void stats_begin(Font *f, Shader *text, Shader *simple);
void stats_end(void);
void stats_end_of_frame(void);
void stats_render(int width, int height);

#endif
