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
    strcpy(meta->artist, "N/A");
    strcpy(meta->album, "N/A");
    strcpy(meta->title, "N/A");
    strcpy(meta->date, "N/A");
    strcpy(meta->genre, "N/A");
    for (int i = 0; i < 2; i++) {
        meta->track[i] = 0;
        meta->disc[i] = 0;
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

        // Check for the specific tags and assign values to variables accordingly
        if (_strnicmp(tagString, "ARTIST=", strlen("ARTIST=")) == 0) {
            strcpy(flac_meta->artist, strchr(tagString, '=') + 1);
            toLowerCase(flac_meta->artist);
        }
        else if (_strnicmp(tagString, "ALBUM=", 6) == 0) {
            strcpy(flac_meta->album, strchr(tagString, '=') + 1);
            toLowerCase(flac_meta->album);
        }
        else if (_strnicmp(tagString, "TITLE=", 6) == 0) {
            strcpy(flac_meta->title, strchr(tagString, '=') + 1);
            toLowerCase(flac_meta->title);
        }
        else if (_strnicmp(tagString, "GENRE=", 6) == 0) {
            strcpy(flac_meta->genre, strchr(tagString, '=') + 1);
        }
        else if (_strnicmp(tagString, "DATE=", 5) == 0) {
            strcpy(flac_meta->date, strchr(tagString, '=') + 1);
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
static void toLowerCase(char* str)
{
    // "function words" that are converted to lowercase
    char* words[] = {
        " In ", " Is ", " The ", " With ", " A ", " As ", " At ",
        " And ", " For ", " From ", " To ", " Or ", " Of ", " On "
    };

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
        }
    }
}

/*
 *----------------------------------------------------------------------
 *
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

    // If the artist name starts with "The ", move The to the end. (The Band -> Band, The)
    if (strncmp(meta->artist, "The ", strlen("The ")) == 0) {
        artistNoTHE = strchr(meta->artist, ' ') + 1;
        int result = snprintf(folder_name, MAX_LENGTH, "%s/%s, The", dest_dir, artistNoTHE);

        if (result < 0 || result >= MAX_LENGTH) {
            fprintf(stderr, "Error formatting folder name.\n");
            return 0;
        }
    } else {
        // If the artist name doesn't start with "The ", just copy the artist name
        int result = snprintf(folder_name, MAX_LENGTH, "%s/%s", dest_dir, meta->artist);

        if (result < 0 || result >= MAX_LENGTH) {
            fprintf(stderr, "Error formatting folder name.\n");
            return 0;
        }
    }

    printf("%s\n", folder_name);

    // Use the mkdir function to create a new directory
    if (mkdir(folder_name, FULL_PERMISSIONS) == 0) {
        printf("Directory '%s' created successfully.\n", folder_name);
        return 1;
    } else {
        perror("Error creating directory");
        return 0;
    }
}