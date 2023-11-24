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
#define MAX_LENGTH 64
#define FULL_PERMISSIONS 0777

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
static void initialize_audioMetaData(audioMetaData* meta, const char* filename, char* ext);

// Display the members of an audioMetaData struct
void print_audioMetaData(audioMetaData* meta);

// Helper function to validate FLAC metadata
static bool validateFlacMeta(BYTE** buffer, int* offset, DWORD length);

// Helper function to parse FLAC metadata
static bool parseFlacMeta(audioMetaData* flac_meta, BYTE* buffer, int size);

// Function to read FLAC file and populate metadata
audioMetaData* get_audioMetaData_flac(const char* filename);

// Function to read MP3 file and populate metadata
audioMetaData* get_audioMetaData_mp3(const char* filename);

// Converts certain words to lowercase (for use in titles, artist and album names)
static void toLowerCase(char* str);

// Creates an artist folder, special case formatted (The Band -> Band, The)
int create_artist_folder(audioMetaData* meta, char* dest_dir);