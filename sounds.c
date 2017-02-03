#include "sounds.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

uint32_t sound_count = 0;
char **sounds = NULL;
static char sound_directory[MAX_PATH];

void sounds_initialize(const char *folder)
{
	char directory[MAX_PATH];

	snprintf(sound_directory, sizeof(sound_directory), "%s/", folder);
	snprintf(directory, sizeof(directory), "%s/*", folder);

	// Open the folder and get a list of files inside it.
	WIN32_FIND_DATA find_data;
	HANDLE file = FindFirstFile(directory, &find_data);

	if (file == INVALID_HANDLE_VALUE) {

		// Something went wrong, the folder doesn't exist?
		// TODO: Handle better lol!
		printf("Could not find first file in directory '%s'!\n", directory);
		return;
	}

	// Count the number of sounds in the directory. Skip everything that isn't a .wav file (or at least pretend to be one).
	sound_count = 0;

	do {
		// Skip directories.
		if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;
		}

		// Make sure the filename ends with .wav.
		size_t len = strlen(find_data.cFileName);

		if (len > 4 && strcmp(&find_data.cFileName[len - 4], ".wav") == 0) {
			++sound_count;
		}
	} while (FindNextFile(file, &find_data) != 0);

	// Allocate an array for the sound list...
	sounds = malloc(sound_count * sizeof(char *));
	
	// ...and add each sound in the sound folder to it.
	file = FindFirstFile(directory, &find_data);

	uint32_t counter = 0;

	do {
		if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			continue;
		}

		size_t len = strlen(find_data.cFileName);

		if (len > 4 && strcmp(&find_data.cFileName[len - 4], ".wav") == 0) {
			// Remove the file extension and add the file to the sound list.
			find_data.cFileName[len - 4] = 0;

			sounds[counter] = malloc(len - 3);
			strncpy(sounds[counter], find_data.cFileName, len - 3);

			++counter;
		}
	} while (FindNextFile(file, &find_data) != 0);
	
	FindClose(file);

	printf("Loaded %u sounds in directory %s.\n", sound_count, sound_directory);
}

void sounds_shutdown(void)
{
	*sound_directory = 0;

	// Free the sound array and its contents.
	for (uint32_t i = 0; i < sound_count; ++i) {
		free(sounds[i]);
	}

	free(sounds);

	sound_count = 0;
	sounds = NULL;
}

void sounds_play(const char *sound)
{
	char path[MAX_PATH];

	printf("Playing sound '%s'.", sound);

	snprintf(path, sizeof(path), "%s/%s.wav", sound_directory, sound);
	PlaySound(path, NULL, SND_ASYNC);
}
