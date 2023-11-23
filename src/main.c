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
#include "../include/config.h"
#include "../include/filelist.h"
#include "../include/metadata.h"

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
            tag = get_audioMetaData_flac(fileList[i]);
        }
        else if (!strcmp(ftype, "mp3")) {
            tag = get_audioMetaData_mp3(fileList[i]);
        }
        else
            printf("Error: Unsupported file type *.%s. ", ftype);

        if (tag == NULL) {
            printf("Skipping [%s]\n\n", fileList[i]);
        }
        else {
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
