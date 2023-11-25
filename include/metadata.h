/**
 * @file audio_metadata.h
 * @brief Declarations for audio metadata handling.
 *
 * This header file declares data types, structures, and enums related to audio metadata.
 * It provides the necessary declarations for functions that handle audio metadata,
 * including reading, parsing, updating, and printing metadata information. Additionally,
 * it defines constants and structures needed for working with audio metadata in different
 * file formats such as MP3 and FLAC.
 *
 * @author [Jayson Gazeley]
 * @date [11/25/2023]
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

/**
 * @brief Initializes an audioMetaData structure with default values.
 *
 * This function initializes an audioMetaData structure by setting default values
 * for various metadata fields. It is typically called before populating the structure
 * with actual metadata from a file.
 *
 * @param meta Pointer to the audioMetaData structure to be initialized.
 * @param filename The path to the audio file associated with the metadata.
 * @param ext The file extension of the audio file.
 */
static void initialize_audioMetaData(audioMetaData* meta, const char* filename, char* ext);


/**
 * @brief Prints the audio metadata to the console.
 *
 * This function prints the values of various audio metadata fields, including
 * Artist, Album, Title, Date, Genre, Track, Disc, and Source file, to the console.
 *
 * @param meta Pointer to the audioMetaData structure containing metadata to be printed.
 */
void print_audioMetaData(audioMetaData* meta);


/**
 * @brief Validates the FLAC metadata block by checking for the presence of "libFLAC".
 *
 * This function checks if the FLAC metadata block contains the expected "libFLAC"
 * identifier, validating the integrity of the metadata. It advances the buffer
 * pointer and updates the offset if the identifier is found.
 *
 * @param buffer Pointer to the buffer containing the FLAC metadata block.
 *               The pointer is advanced if validation is successful.
 * @param offset Pointer to the offset tracking the position in the FLAC metadata block.
 *               The offset is updated if validation is successful.
 * @param length The length of the identifier in the metadata block.
 *
 * @return Returns true if validation is successful, indicating the presence of "libFLAC",
 *         and false if validation fails.
 *
 * @note The function dynamically allocates memory for the identifier string and frees it
 *       before returning. The caller is responsible for freeing the memory
 *       allocated for the identifier string.
 */
static bool validateFlacMeta(BYTE** buffer, int* offset, DWORD length);


/**
 * @brief Updates metadata in the file.
 *
 * This function updates specified metadata (ARTIST, ALBUM, TITLE) in the provided
 * audioMetaData structure and writes the changes back to the file. It uses the type
 * parameter to determine which field to update.
 *
 * @param flac_meta Pointer to the audioMetaData structure containing metadata.
 * @param type The type of metadata to update (ARTIST, ALBUM, TITLE).
 * @param tagString The tag string containing the new metadata.
 * @param totalBytes The total number of bytes processed in the FLAC metadata block.
 *
 * @note The function modifies the metadata in the audioMetaData structure and writes
 *       the changes to the file. It also converts the updated field to lowercase,
 *       if applicable.
 */
static void updateMetadata(struct audioMetaData* flac_meta, enum MetadataType type, const char* tagString, int totalBytes);


/**
 * @brief Replaces specified characters in a string with hyphens.
 *
 * This function iterates through each character in the input string and replaces
 * occurrences of '/', '\\', and '?' with hyphens ('-'). It modifies the input
 * string in-place.
 *
 * @param str Pointer to the string in which characters are to be replaced.
 */
static void replaceChars(char *str);


/**
 * @brief Parses the FLAC metadata block and updates the audioMetaData structure.
 *
 * This function is responsible for parsing the FLAC metadata block, extracting
 * relevant information, and updating the audioMetaData structure accordingly.
 * It specifically handles tags such as ARTIST, ALBUM, TITLE, GENRE, DATE,
 * TRACKNUMBER, TRACKTOTAL, DISCNUMBER, DISCTOTAL, etc.
 *
 * @param flac_meta Pointer to the audioMetaData structure to be updated.
 * @param buffer Pointer to the buffer containing the FLAC metadata block.
 * @param size Size of the FLAC metadata block.
 *
 * @return Returns true on successful parsing and updating of metadata, and false
 *         on any errors during the process.
 *
 * @note The function dynamically allocates memory for the tagString and frees it
 *       before returning. The caller is responsible for freeing the memory
 *       allocated for the tagString.
 */
static bool parseFlacMeta(audioMetaData* flac_meta, BYTE* buffer, int size);


/**
 * @brief Retrieves metadata for a FLAC file.
 *
 * This function allocates memory for an audioMetaData structure, reads the header
 * of a FLAC file to check for validity, and processes the FLAC metadata blocks,
 * specifically the Vorbis comment block. It returns a pointer to the created
 * audioMetaData structure.
 *
 * @param filename The path to the FLAC file from which metadata is to be retrieved.
 *
 * @return Returns a pointer to the allocated audioMetaData structure on success,
 *         or NULL on failure. Failure can occur if the file cannot be opened,
 *         is not a valid FLAC file, or if memory allocation fails. Errors are
 *         printed to stderr.
 *
 * @note The caller is responsible for freeing the allocated memory using free()
 *       when done using the audioMetaData structure.
 */
audioMetaData* get_audioMetaData_flac(const char* filename);


/**
 * @brief Retrieves metadata for an MP3 file with ID3 tags.
 *
 * This function allocates memory for an audioMetaData structure, reads the header
 * of an MP3 file to check for the presence of ID3 tags, and initializes default
 * values for artist, album, and other metadata fields. It returns a pointer to the
 * created audioMetaData structure.
 *
 * @param filename The path to the MP3 file from which metadata is to be retrieved.
 *
 * @return Returns a pointer to the allocated audioMetaData structure on success,
 *         or NULL on failure. Failure can occur if the file cannot be opened,
 *         is not an ID3v2 MP3 file, or if memory allocation fails. Errors are
 *         printed to stderr.
 *
 * @note The caller is responsible for freeing the allocated memory using free()
 *       when done using the audioMetaData structure.
 */
audioMetaData* get_audioMetaData_mp3(const char* filename);


/**
 * @brief Converts specified "function words" in a string to lowercase.
 *
 * This function takes a string and converts specific "function words" to lowercase.
 * Function words are defined in the 'words' array and include common words like
 * "In," "Is," "The," etc. The function iterates through the string and converts
 * occurrences of these function words to lowercase.
 *
 * @param str Pointer to the string to be processed. The function modifies this
 *            string in-place.
 *
 * @return Returns the number of conversions (number of function words converted to
 *         lowercase) in the given string.
 */
static int toLowerCase(char* str);


/**
 * @brief Handle error messages by printing them to the standard error stream.
 *
 * This function prints an error message to the standard error stream.
 *
 * @param message The error message to be printed.
 */
void handle_error(const char* message);

/**
 * @brief Create an artist folder in the specified destination directory.
 *
 * This function creates an artist folder in the specified destination directory.
 *
 * @param dest_dir The destination directory where the artist folder should be created.
 * @param artist The name of the artist for whom the folder is created.
 * @param folder_name A buffer to store the full path of the created folder.
 * @return True if the folder creation is successful, false otherwise.
 */
bool create_artist_folder(const char* dest_dir, const char* artist, char* folder_name);

/**
 * @brief Create an album folder in the specified destination directory.
 *
 * This function creates an album folder in the specified destination directory.
 *
 * @param dest_dir The destination directory where the album folder should be created.
 * @param artist The name of the artist for whom the album folder is created.
 * @param album The name of the album for which the folder is created.
 * @param folder_name A buffer to store the full path of the created folder.
 * @return True if the folder creation is successful, false otherwise.
 */
bool create_album_folder(const char* dest_dir, const char* artist, const char* album, char* folder_name);

/**
 * @brief Reformat the file name and path.
 *
 * This function reformats the file name and path by replacing specific characters.
 *
 * @param meta The audioMetaData structure containing file information.
 * @param folder_name The name of the folder in which the file is stored.
 */
void reformat_file_path(audioMetaData* meta, const char* folder_name);

/**
 * @brief Create the album folder structure.
 *
 * This function creates the album folder structure based on the provided metadata.
 *
 * @param meta The audioMetaData structure containing file information.
 * @param dest_dir The destination directory where the folder structure should be created.
 * @return True if the folder structure creation is successful, false otherwise.
 */
bool create_folder_structure(audioMetaData* meta, const char* dest_dir);
