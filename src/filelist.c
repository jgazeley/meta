#include "../include/filelist.h"

char**
get_filenames(path, count/*, ext*/)
    char* path;
    int* count;
    // const char* ext;
{
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

char*
get_file_extension(filename)
    const char* filename;
{
    return strrchr(filename, '.') + 1;
}

void
print_filenames(fileList, fcount)
    char* fileList[];
    int fcount;
{
    for (int i = 0; i < fcount; i++) {
        printf("File #%2d | %s\n", i, fileList[i]);
    }   
}


// int main(int argc, char* argv[]) {
//     if (argc != 2) {
//         printf("Usage: %s <path>\n", argv[0]);
//         return 1;
//     }

//     int count = 0;
//     char* src = argv[1];
//     char** list = get_filenames(src, &count);
//     print_filenames(list, count);

//     // Free allocated memory for each file name
//     for (int i = 0; i < count; i++) {
//         free(list[i]);
//     }
//     // Free memory for the array of strings
//     free(list);

//     return 0;
// }