#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

bool file_exists(const char *path)
{
	FILE *f = fopen(path, "r");

	if (f == NULL) {
		return false;
	}

	fclose(f);
	return true;
}

bool string_ends_with(const char *str, const char *suffix)
{
	size_t len = strlen(str);
	size_t suffix_len = strlen(suffix);

	return (len >= suffix_len && strcmp(suffix, &str[len - suffix_len]) == 0);
}

char *string_duplicate(const char *str)
{
	size_t len = strlen(str);
	char *buf = malloc(len + 1);

	strcpy(buf, str);
	buf[len] = 0;

	return buf;
}

void for_each_item_in_directory(const char *directory, bool is_item_directory, void (*action)(char *name))
{
	char path[MAX_PATH];
	snprintf(path, sizeof(path), "%s/*", directory);

	WIN32_FIND_DATA find_data;
	HANDLE file = FindFirstFile(path, &find_data);

	if (file == INVALID_HANDLE_VALUE) {
		// Something went wrong, the folder doesn't exist?
		return;
	}

	do {
		// Only process either files or directories, according to the is_item_directory flag.
		if (is_item_directory == ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)) {

			// Ignore hidden items and parent folders.
			if (find_data.cFileName[0] == '.') {
				continue;
			}

			action(find_data.cFileName);
		}
	} while (FindNextFile(file, &find_data) != 0);

	FindClose(file);
}
