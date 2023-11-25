#include "../include/metadata.h"

/*
 *----------------------------------------------------------------------
 *
 * initializeMeta --
 *
 * @brief Initializes the fields of an audioMetaData structure with default values.
 *
 * @param meta A pointer to the audioMetaData structure to be initialized.
 * @param filename The path to the associated audio file.
 *
 * This function initializes the fields of the audioMetaData structure with default values.
 * The default values include "N/A" for string fields, and 0 for integer fields in track and disc arrays.
 *
 * @return None.
 *
 *----------------------------------------------------------------------
 */
static void initialize_audioMetaData(audioMetaData* meta, const char* filename, char* ext)
{
    // Initialize struct member values
    strcpy(meta->pathname, filename);
    strcpy(meta->fileext, ext);
    strcpy(meta->artist, "");
    strcpy(meta->album, "");
    strcpy(meta->title, "");
    strcpy(meta->date, "");
    strcpy(meta->genre, "");
    for (int i = 0; i < 2; i++) {
        meta->track[i] = 0;
        meta->disc[i] = 0;
    }
    meta->metaPtr = 0;
    for (int i = 0; i < 9; i++) {
        meta->offset[i] = 0;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * print_audioMetaData --
 *
 * @brief Prints the members of an audioMetaData structure.
 *
 * @param meta A pointer to the audioMetaData structure to be printed.
 *
 * This function prints the fields of an audioMetaData structure.
 *
 * @return None.
 *
 *----------------------------------------------------------------------
 */
void print_audioMetaData(audioMetaData* meta)
{
    printf("Artist:  \t%s\n", meta->artist);
    printf("Album:   \t%s\n", meta->album);
    printf("Title:   \t%s\n", meta->title);
    printf("Date:    \t%s\n", meta->date);
    printf("Genre:   \t%s\n", meta->genre);
    printf("Track:   \t%d/%d\n", meta->track[0], meta->track[1]);
    if (meta->disc[0] != 0) {
        printf("Disc:    \t%d/%d\n", meta->disc[0], meta->disc[1]);
    }
    printf("Source file:  \t%s\n\n", meta->pathname);
}

/*
 *----------------------------------------------------------------------
 *
 * validateFlacMeta --
 *
 * @brief Validates FLAC metadata by checking for the presence of the "libFLAC" identifier.
 *
 * @param buffer A pointer to a pointer to the beginning of the FLAC metadata block buffer.
 * @param offset A pointer to an integer representing the offset within the buffer.
 * @param length The length of the FLAC metadata block.
 *
 * This function validates a FLAC metadata block by inspecting the initial string within the buffer.
 * It checks whether the "libFLAC" identifier is present, and if so, it advances the buffer pointer
 * and offset accordingly. In case of a validation failure, it prints an error message.
 *
 * @return Returns true if the metadata block is successfully validated, false otherwise.
 *
 *----------------------------------------------------------------------
 */
static bool validateFlacMeta(BYTE** buffer, int* offset, DWORD length) {
    bool result = true;

    // Allocate memory for the reference libFLAC identifier
    char* id = (char*)malloc(length + 1);   // +1 for the null terminator
    memcpy(id, *buffer, length);
    id[length] = '\0';  // Null terminate the string

    // Check if "libFLAC" is found in the initial string, advance the pointer if true
    if (strstr(id, "libFLAC") != NULL) {
        *buffer += sizeof(DWORD) + length; // Advance the buffer pointer
        *offset += sizeof(DWORD) + length; // Update the offset
    }
    // identifier not found, validation failed
    else {
        printf("Error: Metadata tags missing or corrupt. ");
        result = false; // Set the result to false
    }

    free(id);
    return result;
}

// Function to update metadata in the file
static void updateMetadata(struct audioMetaData* flac_meta, enum MetadataType type, const char* tagString, int totalBytes) {
    const char* tagNames[] = {"ARTIST=", "ALBUM=", "TITLE="};
    size_t tagLengths[] = {strlen("ARTIST="), strlen("ALBUM="), strlen("TITLE=")};

    size_t tagIndex = type;
    size_t tagLength = tagLengths[tagIndex];

    if (_strnicmp(tagString, tagNames[tagIndex], tagLength) == 0) {
        char* targetField;
        switch (type) {
            case Artist:
                targetField = flac_meta->artist;
                break;
            case Album:
                targetField = flac_meta->album;
                break;
            case Title:
                targetField = flac_meta->title;
                break;
            default:
                return; // Invalid type
        }

        strcpy(targetField, strchr(tagString, '=') + 1);
        if (toLowerCase(targetField)) {
            flac_meta->offset[type] = flac_meta->metaPtr + sizeof(int) + totalBytes + tagLength;

            FILE* file;
            if (!(file = fopen(flac_meta->pathname, "r+"))) {
                perror("Error opening file");
            } else {
                if (fseek(file, flac_meta->offset[type], SEEK_SET) != 0) {
                    perror("Error seeking file");
                } else {
                    size_t fieldLength = strlen(targetField);
                    if (fwrite(targetField, sizeof(char), fieldLength, file) != fieldLength) {
                        perror("Error writing metadata to file");
                    }
                }
                fclose(file);
            }
        }
    }
}

static void replaceChars(char *str) {
    while (*str) {
        if (*str == '/' || *str == '\\' || *str == '?') {
            *str = '-';
        }
        str++;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * parseFlacMeta --
 *
 * @brief Parses FLAC metadata from a buffer and populates an audioMetaData structure.
 *
 * @param flac_meta A pointer to the audioMetaData structure to be populated.
 * @param buffer A pointer to the beginning of the FLAC metadata block buffer.
 * @param size The size of the FLAC metadata block.
 *
 * This function reads a FLAC metadata block from the given buffer, validates it using the
 * validateFlacMeta function, and assigns values to the corresponding fields of the audioMetaData
 * structure based on the recognized tags. Supported tags include "ARTIST," "ALBUM," "TITLE," "GENRE,"
 * and "DATE." Track and disc-related tags are ignored in this example. The function dynamically
 * allocates memory for tag strings during processing.
 *
 * @return Returns true if the FLAC metadata is successfully parsed and assigned to flac_meta,
 *         false otherwise.
 *
 *----------------------------------------------------------------------
 */
static bool parseFlacMeta(audioMetaData* flac_meta, BYTE* buffer, int size)
{
    DWORD length = 0;
    char* tagString = NULL;
    int totalBytes = 0;

    // Read 4 bytes and advance the buffer
    memcpy(&length, buffer, sizeof(DWORD));
    buffer += sizeof(DWORD);
    totalBytes += sizeof(DWORD);

    if (!validateFlacMeta(&buffer, &totalBytes, length)) {
        return false;
    }

    // Loop until the entire metadata block is processed
    while (totalBytes < size) {

        // Read 4 bytes as an integer for the next iteration
        memcpy(&length, buffer, sizeof(DWORD));
        buffer += sizeof(DWORD);

        // Reallocate memory for the next tag string
        tagString = (char*)realloc(tagString, length + 1);
        memcpy(tagString, buffer, length);
        tagString[length] = '\0';  // Null terminate the string

        // Check for the specific tags and call the updateMetadata function
        if (_strnicmp(tagString, "ARTIST=", strlen("ARTIST=")) == 0) {
            updateMetadata(flac_meta, Artist, tagString, totalBytes);
        } else if (_strnicmp(tagString, "ALBUM=", strlen("ALBUM=")) == 0) {
            updateMetadata(flac_meta, Album, tagString, totalBytes);
        } else if (_strnicmp(tagString, "TITLE=", strlen("TITLE=")) == 0) {
            updateMetadata(flac_meta, Title, tagString, totalBytes);
        }
        else 
            int tagLength = strlen("GENRE=");
        if (_strnicmp(tagString, "GENRE=", tagLength) == 0) {
            strcpy(flac_meta->genre, strchr(tagString, '=') + 1);
            flac_meta->offset[Genre] = flac_meta->metaPtr + sizeof(int) + totalBytes + tagLength;
        }
        else
            tagLength = strlen("DATE=");
        if (_strnicmp(tagString, "DATE=", tagLength) == 0) {
            strcpy(flac_meta->date, strchr(tagString, '=') + 1);
            flac_meta->offset[Date] = flac_meta->metaPtr + sizeof(int) + totalBytes + tagLength;
        }
        else if (_strnicmp(tagString, "TRACKNUMBER=", 12) == 0 ||
            _strnicmp(tagString, "TRACKTOTAL=", 11) == 0 ||
            _strnicmp(tagString, "TRACK=", 6) == 0 ||
            _strnicmp(tagString, "TOTALTRACKS=", 12) == 0) {
            if (_strnicmp(tagString, "TRACKNUMBER=", 12) == 0 || _strnicmp(tagString, "TRACK=", 6) == 0) {
                flac_meta->track[0] = atoi(strchr(tagString, '=') + 1);
            }
            else {
                flac_meta->track[1] = atoi(strchr(tagString, '=') + 1);
            }
        }
        else if (_strnicmp(tagString, "DISCNUMBER=", 11) == 0 ||
            _strnicmp(tagString, "DISCTOTAL=", 10) == 0 ||
            _strnicmp(tagString, "DISC=", 5) == 0 ||
            _strnicmp(tagString, "TOTALDISCS=", 11) == 0) {
            if (_strnicmp(tagString, "DISCNUMBER=", 11) == 0 || _strnicmp(tagString, "DISC=", 5) == 0) {
                flac_meta->disc[0] = atoi(strchr(tagString, '=') + 1);
            }
            else {
                flac_meta->disc[1] = atoi(strchr(tagString, '=') + 1);
            }
        }
        // Advance the pointer by length bytes
        buffer += length;
        totalBytes += sizeof(DWORD) + length;
    }

    free(tagString);

    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * get_audioMetaData_flac --
 *
 * @brief Reads FLAC metadata from a file and populates an audioMetaData structure.
 *
 * @param filename The path to the FLAC file.
 *
 * This function reads a FLAC file, verifies the FLAC header, and processes metadata blocks.
 * It allocates memory for the metadata block buffer, reads the block, and parses it using the
 * parseFlacMeta function. The resulting metadata is stored in the audioMetaData structure.
 *
 * @return A pointer to the audioMetaData structure containing the parsed metadata,
 *         or NULL if there was an error in opening the file or parsing the metadata.
 *
 *----------------------------------------------------------------------
 */
audioMetaData* get_audioMetaData_flac(const char* filename)
{
    audioMetaData* flac_meta = (audioMetaData*)malloc(sizeof(audioMetaData));
    FILE* file;                     // the FLAC file containing metadata
    BYTE header[sizeof(int)];       // for each 4 byte header containing the type and size of the following block
    BYTE* buffer = NULL;            // a buffer for the relevant metadata
    int bytesRead;                  // used to verify fread() function is successful
    bool finalBlock = false;        // true if the current block is the final one (MSB of header is set)

    // Open the FLAC file for reading
    if (!(file = fopen(filename, "rb"))) {
        perror("Error opening file");
        return NULL;
    }

    // Check if the first 4 bytes are 'fLaC' indicating a valid flac file
    bytesRead = fread(header, sizeof(BYTE), 4, file);
    if (bytesRead < 4 || memcmp(header, "fLaC", 4) != 0) {
        printf("Error: Not a real FLAC file. ");
        goto cleanup;
    }

    // Initialize default struct values for artist/album...etc
    initialize_audioMetaData(flac_meta, filename, "flac");

    // Get the block type and size from each header
    // Check the MSB of the first byte. If set, this is the final block
    while (!finalBlock) {
        bytesRead = fread(header, sizeof(BYTE), 4, file);
        if (bytesRead < sizeof(int)) {
            printf("Error: Data missing or corrupt. ");
            goto cleanup;
        }

        finalBlock = header[0] & 0x80;

        int blockType = header[0] & 0x7F;
        int blockSize = (header[1] << 16) | (header[2] << 8) | header[3];

        if (blockType == FLAC_META_VORBIS_COMMENT) {
            // Track the offset of the comment block
            flac_meta->metaPtr = ftell(file);
            // Allocate buffer for metadata block
            if (!(buffer = (BYTE*)malloc(blockSize))) {
                goto cleanup;
            }

            // Read metadata block into the buffer
            bytesRead = fread(buffer, sizeof(BYTE), blockSize, file);
            if (bytesRead < blockSize) {
                printf("Error: Couldn't read tag info. ");
                goto cleanup;
            }

            if (!(parseFlacMeta(flac_meta, buffer, blockSize))) {
                printf("FLAC file could not be parsed.");
                goto cleanup;
            }

            // Free the buffer after processing
            free(buffer);
        }
        else {
            // Advance the file pointer to the next header
            fseek(file, blockSize, SEEK_CUR);
        }
    }

    fclose(file);
    return flac_meta;

cleanup:
    fclose(file);
    free(buffer);
    free(flac_meta);
    return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * get_audioMetaData_mp3 --
 *
 * @brief Reads MP3 ID3v2 metadata from a file and populates an audioMetaData structure.
 *
 * @param filename The path to the MP3 file.
 *
 * This function reads a MP3 file, verifies the ID3 header, and processes metadata blocks.
 * It allocates memory for the metadata block buffer, reads the block, and parses it using the
 * parseMP3Meta function. The resulting metadata is stored in the audioMetaData structure.
 *
 * @return A pointer to the audioMetaData structure containing the parsed metadata,
 *         or NULL if there was an error in opening the file or parsing the metadata.
 *
 *----------------------------------------------------------------------
 */
audioMetaData* get_audioMetaData_mp3(const char* filename)
{
    audioMetaData* mp3_meta = (audioMetaData*)malloc(sizeof(audioMetaData));
    FILE* file;                     // the MP3 file containing metadata
    BYTE header[10];                // for the 10 byte header containing the ID3 tag info 
    BYTE* buffer;                   // a buffer for the relevant metadata

    // Open the MP3 file for reading
    if (!(file = fopen(filename, "rb"))) {
        char errmsg[256];
        sprintf(errmsg, "fopen(%s, \"rb\")", filename);
        perror(errmsg);
        return NULL;
    }

    // Check if the first 3 bytes are 'ID3' indicating an mp3 file with ID3 tags
    fread(header, sizeof(BYTE), 10, file);
    if (memcmp(header, "ID3", 3) != 0) {
        printf("Error: Not an ID3v2 mp3 file. ");
        fclose(file);
        free(mp3_meta);
        return NULL;
    }

    // Initialize default struct values for artist/album...etc
    initialize_audioMetaData(mp3_meta, filename, "mp3");

    fclose(file);
    return mp3_meta;
}

/*
 *----------------------------------------------------------------------
 *
 * toLowerCase --
 *
 * @brief Converts specific words within a string to lowercase, preserving spaces.
 *
 * @param str A pointer to the string to be converted to lowercase.
 *
 * This function iterates through a list of words and converts occurrences of those words
 * within the given string to lowercase, while preserving spaces. The function modifies
 * the input string in place.
 *
 *----------------------------------------------------------------------
 */
#include <string.h>
#include <ctype.h>

static int toLowerCase(char* str)
{
    // "function words" that are converted to lowercase
    char* words[] = {
        " In ", " Is ", " The ", " With ", " A ", " As ", " At ",
        " And ", " For ", " From ", " To ", " Or ", " Of ", " On "
    };

    int conversionCount = 0;  // Counter for the number of conversions

    // Iterate through each word and convert to lowercase
    for (int i = 0; i < sizeof(words) / sizeof(char*); i++) {
        char* word = words[i];
        char* start = str;
        while ((start = strstr(start, word)) != NULL) {
            char* end = start + strlen(word);
            while (start < end) {
                *start = tolower(*start);
                start++;
            }
            conversionCount = 1;  // Set the flag to indicate at least one conversion
        }
    }

    return conversionCount;
}

/*
 * create_artist_folder --
 *
 * @brief Creates a folder for an artist, handling special cases for names starting with "The ".
 *
 * @param artist A pointer to the artist name for which the folder is to be created.
 *
 * This function generates a folder name for the given artist, considering special cases where
 * the artist name starts with "The ". It creates a new directory with the formatted folder name.
 * The function returns 1 on successful directory creation, and 0 otherwise.
 *
 *----------------------------------------------------------------------
 */
int create_artist_folder(audioMetaData* meta, char* dest_dir)
{
    char folder_name[MAX_LENGTH] = "";
    char* artistNoTHE = NULL;
    if (strlen(meta->artist) == 0 || strlen(meta->album) == 0) {
        fprintf(stderr, "Error: Artist or album field is blank. ", meta->pathname);
        return 1;
    }

    // If the artist name starts with "The ", move The to the end. (The Band -> Band, The)
    if (strncmp(meta->artist, "The ", strlen("The ")) == 0) {
        artistNoTHE = strchr(meta->artist, ' ') + 1;
        int result = snprintf(folder_name, MAX_LENGTH, "%s/%s, The", dest_dir, artistNoTHE);

        if (result < 0 || result >= MAX_LENGTH) {
            fprintf(stderr, "Error formatting folder name 1. ");
            return 0;
        }
    } else {
        // If the artist name doesn't start with "The ", just copy the artist name
        int result = snprintf(folder_name, MAX_LENGTH, "%s/%s", dest_dir, meta->artist);

        if (result < 0 || result >= MAX_LENGTH) {
            fprintf(stderr, "Error formatting folder name 2. ");
            return 0;
        }
    }

    // Check if the artist folder already exists
    if (_access(folder_name, 0) == -1) {
         // Use the _mkdir function to create a new directory
        if (_mkdir(folder_name) != 0) {
            perror("Error creating directory");
            return 1;
        }
    }

    strcat(folder_name, "/"); strcat(folder_name, meta->album);
    // Check if the album folder already exists
    if (_access(folder_name, 0) == -1) {
         // Use the _mkdir function to create a new directory
        if (_mkdir(folder_name) != 0) {
            perror("Error creating directory");
            return 1;
        }
    }

    // Reformat the file name and path
    if (strchr(meta->title, '/') || strchr(meta->title, '\\') || strchr(meta->title, '?')) {
        replaceChars(meta->title);
    }
    sprintf(meta->pathname, "%s/%02d. %s.%s", folder_name, meta->track[0], meta->title, meta->fileext);
    return 0;
}