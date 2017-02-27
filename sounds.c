#include "sounds.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

static char sound_directory[MAX_PATH];
static struct sound_folder_t *first_folder;
static uint32_t folder_count;

static char json[131072]; // JSON response which contains all the sounds.

// ----------------------------------------------------------------------
 
static void sounds_process_subdirectory(char *directory);
static void sounds_count_wav_files(char *file_name);
static void sounds_add_sound_to_list(char *file_name);
static void sounds_format_json(void);

// ----------------------------------------------------------------------

void sounds_initialize(const char *folder)
{
	folder_count = 0;

	// Cache the sound folder name for later use.
	strncpy(sound_directory, folder, sizeof(sound_directory));

	// Load all the sounds from the subdirectories within the main sound folder.
	for_each_item_in_directory(folder, true, sounds_process_subdirectory);

	// Now that we have the list of sounds, format the JSON response containing all the sounds and cache it.
	sounds_format_json();
}

void sounds_shutdown(void)
{
	// Free all the things!
	for (struct sound_folder_t *folder = first_folder, *tmp; folder != NULL; folder = tmp) {

		tmp = folder->next;

		for (uint32_t i = 0; i < folder->sound_count; ++i) {
			free(folder->sounds[i]);
		}

		free(folder->sounds);
		free(folder->name);
		free(folder);
	}

	*sound_directory = 0;
	folder_count = 0;
	first_folder = NULL;
}

void sounds_play(const char *category, const char *sound)
{
	char path[MAX_PATH];
	snprintf(path, sizeof(path), "%s/%s/%s.wav", sound_directory, category, sound);

	// Use the sound player app if it is available and has a named pipe open.
	HANDLE pipe = CreateFile("\\\\.\\pipe\\soundBoardPipe", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (pipe != INVALID_HANDLE_VALUE) {

		WriteFile(pipe, path, strlen(path), NULL, NULL);
		CloseHandle(pipe);
	}

	// Use an external sound player if it is available.
	else if (file_exists("SoundPlayer.exe")) {

		char command[MAX_PATH + 24];
		snprintf(command, sizeof(command), "start SoundPlayer.exe \"%s\"", path);
		
		system(command);
	}

	// Otherwise fall back to PlaySound.
	else {
		PlaySound(path, NULL, SND_ASYNC | SND_NOWAIT);
	}
}

const char *sounds_get_json_list(void)
{
	return json;
}

static void sounds_process_subdirectory(char *directory)
{
	++folder_count;

	// Create a folder entry for the subdirectory.
	struct sound_folder_t *folder = malloc(sizeof(*folder));
	memset(folder, 0, sizeof(*folder));

	folder->name = string_duplicate(directory);

	// Add the folder to the list of subdirectories.
	if (first_folder != NULL) {
		folder->next = first_folder;
	}

	first_folder = folder;

	// Count all the .wav files in the folder.
	char path[MAX_PATH];
	snprintf(path, sizeof(path), "%s/%s", sound_directory, directory);

	for_each_item_in_directory(path, false, sounds_count_wav_files);

	// Create an array for the names of the sound files and populate it.
	folder->sounds = malloc(folder->sound_count * sizeof(char *));
	folder->sound_count = 0;

	for_each_item_in_directory(path, false, sounds_add_sound_to_list);

	// And we're done!
	printf("Found %u sound files in folder '%s'.\n", folder->sound_count, folder->name);
}

static void sounds_count_wav_files(char *file_name)
{
	// Ignore all non-wav files.
	if (!string_ends_with(file_name, ".wav")) {
		return;
	}

	first_folder->sound_count++;
}

static void sounds_add_sound_to_list(char *file_name)
{
	// Ignore all non-wav files.
	if (!string_ends_with(file_name, ".wav")) {
		return;
	}

	// Strip the file extension.
	file_name[strlen(file_name) - 4] = 0;

	// Add the sound to the sound array.
	first_folder->sounds[first_folder->sound_count] = string_duplicate(file_name);
	first_folder->sound_count++;
}

static void sounds_format_json(void)
{
	size_t len = 0, c = 0;

	// TODO: Careful with buffer overflows, idiot!
	len += snprintf(&json[len], sizeof(json) - len, "{\n\t\"folders\":[\n\t\t");

	for (struct sound_folder_t *folder = first_folder; folder != NULL; folder = folder->next) {

		++c;

		len += snprintf(&json[len], sizeof(json) - len, "{\n\t\t\t\"name\": \"%s\",\n\t\t\t", folder->name);
		len += snprintf(&json[len], sizeof(json) - len, "\"sounds\": [\n\t\t\t");

		// Add each sound to the JSON.
		if (folder->sound_count != 0) {
			for (uint32_t i = 0; i < folder->sound_count; ++i) {
				len += snprintf(&json[len], sizeof(json) - len, "\"%s\"%s", folder->sounds[i], (i < folder->sound_count - 1 ? ", " : ""));
			}
		}

		len += snprintf(&json[len], sizeof(json) - len, "\n\t\t\t]");
		len += snprintf(&json[len], sizeof(json) - len, "\n\t\t}%s\n\t\t", c < folder_count ? "," : "");
	}

	len += snprintf(&json[len], sizeof(json) - len, "\n\t]\n}");
}
