#include "main.h"
#include "sounds.h"
#include <time.h>

int main(void)
{
	// Seed RNG.
	srand(time(NULL));

	// Load the sounds in the sound folder.
	sounds_initialize("statusd112");

	bool sound_played = false;

	for (;;) {

		if (!sound_played) {
			sounds_play(sounds[rand() % sound_count]);
			sound_played = true;
		}

		Sleep(100);
	}

	// Cleanup and free memory.
	sounds_shutdown();

	return 0;
}
