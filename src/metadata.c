#include "../include/metadata.h"

static void
initialize_audioMetaData(meta, filename, ext)
    audioMetaData* meta;
    const char* filename;
    char* ext;
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

void
print_audioMetaData(meta)
    audioMetaData* meta;
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

static bool
validateFlacMeta(buffer, offset, length)
    BYTE** buffer;
    int* offset;
    DWORD length;
{
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

static void
updateMetadata(flac_meta, type, tagString, totalBytes)
    struct audioMetaData* flac_meta;
    enum MetadataType type;
    const char* tagString;
    int totalBytes;
{
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
                perror("Error : Couln't open file");
            } else {
                if (fseek(file, flac_meta->offset[type], SEEK_SET) != 0) {
                    perror("Error : Couldn't seek file");
                } else {
                    size_t fieldLength = strlen(targetField);
                    if (fwrite(targetField, sizeof(char), fieldLength, file) != fieldLength) {
                        perror("Error : Couldn't write metadata to file");
                    }
                }
                fclose(file);
            }
        }
    }
}

static void
replaceChars(str)
    char* str;
{
    while (*str) {
        if (*str == '/' || *str == '\\' || *str == '?') {
            *str = '-';
        }
        str++;
    }
}

static bool
parseFlacMeta(flac_meta, buffer, size)
    audioMetaData* flac_meta;
    BYTE* buffer;
    int size;
{
    DWORD length = 0;              // Stores the length of the current metadata block
    char* tagString = NULL;        // Pointer to dynamically allocated memory for the current tag string
    int totalBytes = 0;            // Counter for the total number of bytes processed in the metadata
    int tagLength = 0;             // Stores the length of specific tag strings

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

        tagLength = strlen("GENRE=");
        if (_strnicmp(tagString, "GENRE=", tagLength) == 0) {
            strcpy(flac_meta->genre, strchr(tagString, '=') + 1);
        }

        tagLength = strlen("DATE=");
        if (_strnicmp(tagString, "DATE=", tagLength) == 0) {
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

audioMetaData*
get_audioMetaData_flac(filename)
    const char* filename;
{
    audioMetaData* flac_meta = (audioMetaData*)malloc(sizeof(audioMetaData));
    FILE* file;                     // the FLAC file containing metadata
    BYTE header[sizeof(int)];       // for each 4 byte header containing the type and size of the following block
    BYTE* buffer = NULL;            // a buffer for the relevant metadata
    int bytesRead;                  // used to verify fread() function is successful
    bool finalBlock = false;        // true if the current block is the final one (MSB of header is set)

    // Open the FLAC file for reading
    if (!(file = fopen(filename, "rb"))) {
        perror("Error : Couldn't open the file");
        return NULL;
    }

    // Check if the first 4 bytes are 'fLaC' indicating a valid flac file
    bytesRead = fread(header, sizeof(BYTE), 4, file);
    if (bytesRead < 4 || memcmp(header, "fLaC", 4) != 0) {
        handle_error("Not a real FLAC file.");
        goto cleanup;
    }

    // Initialize default struct values for artist/album...etc
    initialize_audioMetaData(flac_meta, filename, "flac");

    // Get the block type and size from each header
    // Check the MSB of the first byte. If set, this is the final block
    while (!finalBlock) {
        bytesRead = fread(header, sizeof(BYTE), 4, file);
        if (bytesRead < sizeof(int)) {
            handle_error("Data missing or corrupt.");
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
                handle_error("Couldn't read tag info.");
                goto cleanup;
            }

            if (!(parseFlacMeta(flac_meta, buffer, blockSize))) {
                handle_error("FLAC file could not be parsed.");
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

audioMetaData*
get_audioMetaData_mp3(filename)
    const char* filename;
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
        handle_error("Not an ID3v2 mp3 file.");
        fclose(file);
        free(mp3_meta);
        return NULL;
    }

    // Initialize default struct values for artist/album...etc
    initialize_audioMetaData(mp3_meta, filename, "mp3");

    fclose(file);
    return mp3_meta;
}

static int
toLowerCase(str)
    char* str;
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

void
handle_error(message)
    const char* message;
{
    fprintf(stderr, "Error : %-30s ", message);
}

bool
create_artist_folder(dest_dir, artist, folder_name)
    const char* dest_dir;
    const char* artist;
    char* folder_name;
{
    int result = snprintf(folder_name, MAX_LENGTH, "%s/%s", dest_dir, artist);
    if (result < 0 || result >= MAX_LENGTH) {
        handle_error("Couldn't format the artist folder name");
        return false;
    }

    // Check if the artist folder already exists
    if (_access(folder_name, 0) == -1) {
        if (_mkdir(folder_name) != 0) {
            perror("Error : Couldn't create artist directory");
            return false;
        }
    }

    return true;
}

bool
create_album_folder(dest_dir, artist, album, folder_name)
    const char* dest_dir;
    const char* artist;
    const char* album;
    char* folder_name;
{
    int result = snprintf(folder_name, MAX_LENGTH, "%s/%s/%s", dest_dir, artist, album);
    if (result < 0 || result >= MAX_LENGTH) {
        handle_error("Couldn't format the album folder name");
        return false;
    }

    if (_access(folder_name, 0) == -1) {
        if (_mkdir(folder_name) != 0) {
            perror("Error : Couldn't create album directory");
            return false;
        }
    }

    return true;
}

void
reformat_file_path(meta, folder_name)
    audioMetaData* meta;
    const char* folder_name;
{
    if (strchr(meta->title, '/') || strchr(meta->title, '\\') || strchr(meta->title, '?')) {
        // Assuming replaceChars is a function that replaces specific characters in a string
        replaceChars(meta->title);
    }
    sprintf(meta->pathname, "%s/%02d. %s.%s", folder_name, meta->track[0], meta->title, meta->fileext);
}

bool
create_folder_structure(meta, dest_dir)
    audioMetaData* meta;
    const char* dest_dir;
{
    char folder_name[MAX_LENGTH] = "";

    if (strlen(meta->artist) == 0 || strlen(meta->album) == 0) {
        handle_error("A field is blank.");
        return false;
    }

    // If the artist name starts with "The ", move "The" to the end.
    if (strncmp(meta->artist, "The ", strlen("The ")) == 0) {
        char temp[MAX_LENGTH];
        snprintf(temp, MAX_LENGTH, "%s, The", meta->artist + strlen("The "));
        strcpy(meta->artist, temp);
    }

    if (!create_artist_folder(dest_dir, meta->artist, folder_name)) {
        return false;
    }

    if (!create_album_folder(dest_dir, meta->artist, meta->album, folder_name)) {
        return false;
    }

    reformat_file_path(meta, folder_name);

    return true;
}