#ifndef __SOUNDS_H
#define __SOUNDS_H

// --------------------------------------------------------------------------------

#include "main.h"

// --------------------------------------------------------------------------------

struct sound_folder_t {
	char *name;
	char **sounds;
	uint32_t sound_count;
	struct sound_folder_t *next;
};

// --------------------------------------------------------------------------------

void sounds_initialize(const char *path);
void sounds_shutdown(void);

void sounds_play(const char *category, const char *sound);

const char *sounds_get_json_list(void);

#endif
