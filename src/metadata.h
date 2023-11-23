/*
 *----------------------------------------------------------------------
 *
 * metadata.h --
 *
 *----------------------------------------------------------------------
 */
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define strcasecmp _stricmp
#define FLAC_META_VORBIS_COMMENT 4

typedef unsigned char BYTE;
typedef unsigned long DWORD;

typedef struct audioMetaData {
    char pathname[_MAX_PATH];
    char fileext[10];
    char artist[32];
    char album[128];
    char title[128];
    char date[5];
    char genre[32];
    int track[2];
    int disc[2];
} audioMetaData;

// Function to initialize audio metadata structure
void initialize_audioMetaData(audioMetaData* meta, const char* filename, char* ext);

// Helper function to validate FLAC metadata
bool validateFlacMeta(BYTE** buffer, int* offset, DWORD length);

// Helper function to parse FLAC metadata
bool parseFlacMeta(audioMetaData* flac_meta, BYTE* buffer, int size);

// Function to read FLAC file and populate metadata
audioMetaData* get_audioMetaData_flac(const char* filename);

// Function to read MP3 file and populate metadata
audioMetaData* get_audioMetaData_mp3(const char* filename);