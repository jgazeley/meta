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

char** get_filenames(char* path, int* count/*, const char* ext*/);
void print_filenames(char* fileList[], int fcount);