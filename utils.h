#ifndef __UTILS_H
#define __UTILS_H

// --------------------------------------------------------------------------------

#include "main.h"

// --------------------------------------------------------------------------------

bool file_exists(const char *path);
bool string_ends_with(const char *str, const char *suffix);
char *string_duplicate(const char *str);
void for_each_item_in_directory(const char *folder, bool is_item_directory, void (*action)(char *name));

#endif
