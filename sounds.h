#ifndef __SOUNDS_H
#define __SOUNDS_H

// --------------------------------------------------------------------------------

#include "main.h"

// --------------------------------------------------------------------------------

void sounds_initialize(const char *path);
void sounds_shutdown(void);

void sounds_play(const char *sound);

// --------------------------------------------------------------------------------

extern uint32_t sound_count;
extern char **sounds;

#endif
