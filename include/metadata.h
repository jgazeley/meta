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
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#define strcasecmp _stricmp
#define FLAC_META_VORBIS_COMMENT 4
#define MAX_LENGTH 128
#define FULL_PERMISSIONS 0777

typedef unsigned char BYTE;
typedef unsigned long DWORD;

typedef struct audioMetaData {
    char pathname[_MAX_PATH];
    char fileext[10];
    char artist[64];
    char album[128];
    char title[128];
    char date[5];
    char genre[32];
    int track[2];
    int disc[2];
    int metaPtr;
    int offset[9];      // enum MetadataField is for the index of this array
} audioMetaData;

typedef enum {
    Artist,       // 0
    Album,        // 1
    Title,        // 2
    Date,         // 3
    Genre,        // 4
    TrackNumber,  // 5
    TotalTracks,  // 6
    DiscNumber,   // 7
    TotalDiscs    // 8
} MetadataField;

// Function to initialize audio metadata structure
static void initialize_audioMetaData(audioMetaData* meta, const char* filename, char* ext);

// Display the members of an audioMetaData struct
void print_audioMetaData(audioMetaData* meta);

// Helper functions to validate FLAC metadata
static bool validateFlacMeta(BYTE** buffer, int* offset, DWORD length);
static void updateMetadata(struct audioMetaData* flac_meta, enum MetadataType type, const char* tagString, int totalBytes);
static void replaceChars(char *str);
static bool parseFlacMeta(audioMetaData* flac_meta, BYTE* buffer, int size);

// Functions to read FLAC/MP3 file and populate metadata
audioMetaData* get_audioMetaData_flac(const char* filename);
audioMetaData* get_audioMetaData_mp3(const char* filename);

// Converts certain words to lowercase (for use in titles, artist and album names)
static int toLowerCase(char* str);

// Creates an artist folder, special case formatted (The Band -> Band, The)
int create_artist_folder(audioMetaData* meta, char* dest_dir);