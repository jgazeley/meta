#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#endif // CONFIG_H

#define MAX_CMD 64

int is_valid_drive_path(const char* path);
int setup(char* src_path, char* dest_path);