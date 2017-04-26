#ifndef __SOUNDS_H
#define __SOUNDS_H

// --------------------------------------------------------------------------------

#include "main.h"

// --------------------------------------------------------------------------------

struct sound_t {
	char *name;					// Name of the sound file
	uint32_t duration;			// Duration of the sound clip in milliseconds
	uint32_t num_times_played;	// How many times has this sound clip been played
	time_t modified;			// Timestamp for when the sound file was last modified
};

// --------------------------------------------------------------------------------

struct sound_folder_t {
	char *name;
	struct sound_t *sounds;
	uint32_t sound_count;
	struct sound_folder_t *next;
};

// --------------------------------------------------------------------------------

// The header of a .wav file.
// Used for determining the length of the audio clip in milliseconds.

#pragma pack(push, 1)

struct wav_header_t {
	uint8_t pad0[22];
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;			// The number of audio data bytes per second.
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint8_t pad40[4];
	uint32_t num_data_bytes;	// The size of the actual audio data in bytes.
};

#pragma pack(pop)

// --------------------------------------------------------------------------------

void sounds_initialize(const char *path);
void sounds_shutdown(void);

void sounds_play(const char *category, const char *sound);

const char *sounds_get_json_list(void);

#endif
