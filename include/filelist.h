#ifndef STDIO_H
#define STDIO_H

#include <stdio.h>

#endif

#ifndef STRING_H
#define STRING_H

#include <string.h>

#endif

#include <dirent.h>
#include <sys/stat.h>

/**
 * @brief Retrieves the list of filenames in a given directory.
 *
 * Opens the specified directory, counts the number of regular files,
 * allocates memory for an array of strings, and populates it with the names of regular files.
 * Memory allocated for filenames must be freed by the caller.
 *
 * @param path The path of the directory.
 * @param count Pointer to an integer to store the count of filenames.
 * @return An array of strings containing the filenames.
 */
char** get_filenames(char* path, int* count/*, const char* ext*/);


/**
 * @brief Extracts and returns the file extension from the given filename.
 *
 * This function parses the input filename to retrieve the file extension.
 * The file extension is defined as the substring following the last period ('.').
 *
 * @param filename The input filename from which to extract the file extension.
 * @return A pointer to the file extension in the original filename, or NULL if no extension is found.
 *
 * @note The returned pointer is part of the original filename and should not be modified or freed.
 * @note If the filename ends with a period, or if no period is found, NULL is returned.
 */
char* get_file_extension(const char* filename);


/**
 * @brief Prints filenames along with their indices from an array of strings.
 *
 * This function takes an array of filenames and the count of filenames,
 * then prints each filename with its corresponding index in the array.
 *
 * @param fileList The array of filenames.
 * @param fcount The count of filenames.
 */
void print_filenames(char* fileList[], int fcount);

