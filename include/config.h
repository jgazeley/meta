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

/**
 * @brief Checks if the given path is a valid drive path on Windows or a valid relative path on Unix/Linux.
 *
 * On Windows, it checks for a valid drive letter and colon followed by an optional backslash or forward slash.
 * On Unix/Linux, it assumes a relative path is always valid.
 *
 * @param path The path to be validated.
 * @return 1 if the path is valid, 0 otherwise.
 */
int is_valid_drive_path(const char* path);


/**
 * @brief Reads source and destination paths from a configuration file or creates it if it doesn't exist.
 *
 * Reads the source and destination paths from a configuration file named "dir.ini." If the file doesn't exist,
 * it creates the file, initializes it with default values, and prompts the user to set the paths using Notepad on Windows.
 * Validates the paths for existence, validity as drive paths, and read and write permissions.
 *
 * @param src_path Buffer to store the source path.
 * @param dest_path Buffer to store the destination path.
 * @return 0 on success, 1 on error.
 */
int setup(char* src_path, char* dest_path);