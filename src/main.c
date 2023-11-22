/*
 *----------------------------------------------------------------------
 *
 * meta.c --
 *
 * @brief Implements functions for reading and processing audio file metadata.
 *
 * This source file contains functions to read and process metadata from audio files,
 * specifically FLAC files. It includes functions for setting up the initial configuration,
 * retrieving file names with a specified extension, validating and parsing FLAC metadata,
 * and processing the main routine for reading and displaying metadata.
 *
 *----------------------------------------------------------------------
 */
#include "metadata.h"
#include <dirent.h>
#include <sys/stat.h>

#define MAX_CMD 64

/*
 *----------------------------------------------------------------------
 *
 * setup --
 *
 * @brief    Creates/reads a configuration file and initializes the source and destination paths.
 *
 * @param    src_path   A pointer to the buffer for the source path.
 * @param    dest_path  A pointer to the buffer for the destination path.
 *
 * @return   This function reads a configuration file ('dir.ini') to set the source and destination
 *           paths. If the file doesn't exist, it creates a new one, initializes it with default values,
 *           prompts the user to set the source and destination paths, and then terminates the program.
 *           If the configuration file exists, it reads the source and destination paths from the file
 *           and updates the corresponding parameters.
 *
 *----------------------------------------------------------------------
 */
void setup(char* src_path, char* dest_path)
{
    FILE* cfg;
    const char* config = "dir.ini";
    char cmd[MAX_CMD] = "";
    char line[_MAX_PATH] = "";
    size_t len = 0;

    // Open the config file if it exists, or create it
    if (!(cfg = fopen(config, "rb"))) {
        printf("Configuration file doesn't exist. Creating...\n");
        if (!(cfg = fopen(config, "wb"))) {
            printf("Error creating file!\n");
            exit(1);
        }

        // Initialize configuration file
        fprintf(cfg, "[Directory]\nSource=\nDestination=\n");
        printf("%s created successfully, please set the ", config);
        printf("source and destination path.\n");
        fclose(cfg);

        // Open configuration file in notepad and terminate this program
        sprintf(cmd, "notepad %s", config);
        system(cmd);
        exit(1);
    }

    // Read source and destination path from the config file
    while (fgets(line, _MAX_PATH, cfg)) {
        if (!strncmp(line, "Source=", strlen("Source="))) {
            strcpy(src_path, strchr(line, '=') + 1);
            len = strlen(src_path);
            if (len > 0 && src_path[len - 1] == '\n')
                src_path[len - 1] = '\0';
        }

        if (!strncmp(line, "Destination=", strlen("Destination="))) {
            strcpy(dest_path, strchr(line, '=') + 1);
            len = strlen(dest_path);
            if (len > 0 && dest_path[len - 1] == '\n')
                dest_path[len - 1] = '\0';
        }
    } fclose(cfg);
}

/*
 *----------------------------------------------------------------------
 *
 * get_filenames --
 *
 * @brief Retrieves an array of strings containing filenames with a specific extension in a given directory.
 *
 * @param path   A pointer to the directory name to be searched.
 * @param count  A pointer to an integer representing the total number of matching files.
 * @param ext    The file extension to match.
 *
 * @return This function opens the specified directory, counts the number of files with the
 *         specified extension, allocates memory for an array of strings, and populates the array
 *         with the full paths of files matching the extension. The caller is responsible for
 *         freeing the memory allocated for the array. If an error occurs (e.g., unable to open the
 *         directory or allocate memory), it returns NULL.
 *
 *----------------------------------------------------------------------
 */
char** get_filenames(char* path, int* count/*, const char* ext*/) {
    DIR* dir;
    struct dirent* entry;
    // char* extension = NULL;
    char** fileList = NULL;
    int index = 0;

    // Open the directory
    if (!(dir = opendir(path))) {
        char errmsg[256];
        sprintf(errmsg, "Source directory %s", path);
        perror(errmsg);
        exit(1);
    }

    // Count the number of files
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Check if it is a regular file
            // extension = strrchr(entry->d_name, '.');
            //if (extension != NULL && strcmp(extension + 1, ext) == 0)
            (*count)++;
        }
    }
    rewinddir(dir);

    // Allocate memory for the array of strings
    fileList = (char**)malloc((*count) * sizeof(char*));
    if (!fileList) {
        perror("Memory allocation error");
        closedir(dir);
        exit(1);
    }

    // Read the names of files and store them in the array
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            // extension = strrchr(entry->d_name, '.');
            //       if (extension != NULL && strcmp(extension + 1, ext) == 0) {
                       // Calculate the length of the new string
            size_t pathLen = strlen(path);
            size_t fileNameLen = strlen(entry->d_name);
            size_t totalLen = pathLen + fileNameLen + 1; // +1 for the null terminator

            // Allocate memory for the file name with path
            fileList[index] = (char*)malloc(totalLen + 1);
            if (!fileList[index]) {
                perror("Memory allocation error");
                closedir(dir);
                exit(1);
            }

            // Concatenate path and filename
            sprintf(fileList[index], "%s/%s", path, entry->d_name);

            index++;
            //          }
        }
    }
    closedir(dir);

    return fileList;
}

// List files found and file number
void print_filenames(char* fileList[], int fcount)
{
    for (int i = 0; i < fcount; i++) {
        printf("File #%2d | %s\n", i, fileList[i]);
    }
    putc('\n', stdout);    
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
void toLowerCase(char* str)
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
 * main --
 *
 * @brief The main entry point of the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 *
 * This function serves as the main entry point of the program. It processes command-line
 * arguments, reads the configuration file, retrieves a list of file names with a specified
 * extension, reads and processes metadata for each file, and then frees the allocated memory.
 *
 * @return Returns 0 on successful execution, or an error code if there's an issue.
 *
 *----------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    printf("Program Start...\n\n");
    const char* program_name = argv[0];
    char* ftype = NULL;                             // file extension
    char src_dir[_MAX_PATH] = "";                   // source folder
    char dest_dir[_MAX_PATH] = "";                  // destination folder
    char** fileList = NULL;                         // list of files (of ftype)
    int fcount = 0;                                 // number of files (of ftype)

    // Check command line arguments
    if (argc != 1) {
        printf("Usage: %s \n\nFIX LATER BACK TO ARGC 2\n", program_name);
        //printf("Example: %s mp3\n", program_name);
        return 1;
    }
    //ftype = argv[1];

    // Read configuration file / initial setup
    setup(src_dir, dest_dir);

    // Get a list of the file names in 'src_dir' with extension 'ftype'
    //fileList = get_filenames(src_dir, &fcount, ftype);l
    fileList = get_filenames(src_dir, &fcount);

    // Read metadata
    for (int i = 0; i < fcount; i++)
    {
        audioMetaData* tag = NULL;
        ftype = strrchr(fileList[i], '.') + 1;
        if (!strcmp(ftype, "flac")) {
            tag = readFlacFile(fileList[i]);
        }
        else if (!strcmp(ftype, "mp3")) {
            tag = readMP3File(fileList[i]);
        }
        else
            printf("Error: Unsupported file type *.%s. ", ftype);

        if (tag == NULL) {
            printf("Skipping [%s]\n\n", fileList[i]);
        }
        else {
            toLowerCase(tag->artist);
            toLowerCase(tag->album);
            toLowerCase(tag->title);
            print_audioMetaData(tag);
        }

        // Free the dynamically allocated memory
        free(tag);
    }

    print_filenames(fileList, fcount);

    // Free allocated memory for each file name
    for (int i = 0; i < fcount; i++) {
        free(fileList[i]);
    }
    // Free memory for the array of strings
    free(fileList);

    printf("Program complete.\n");

    return 0;
}
