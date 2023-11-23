#include "../include/filelist.h"

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

/*
 *----------------------------------------------------------------------
 *
 * print_filenames --
 *
 * @brief Prints the strings in an array of file names.
 *
 * @param fileList An array of file names to be printed.
 * @param fcount   The number of file names in the array.
 *
 * This function prints the file names in the specified array along with their indices.
 *
 * @return None.
 *
 *----------------------------------------------------------------------
 */
void print_filenames(char* fileList[], int fcount)
{
    for (int i = 0; i < fcount; i++) {
        printf("File #%2d | %s\n", i, fileList[i]);
    }
    putc('\n', stdout);    
}