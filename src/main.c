#include "../include/config.h"
#include "../include/filelist.h"
#include "../include/metadata.h"

// Function prototype
void process_file(char* filename, const char* dest_dir, int* successCount);
void print_summary(int successCount, int totalFiles);

int
main(argc, argv)
    int argc;
    char* argv[];
{
    //char* ftype = NULL;                 // file extension
    char src_dir[_MAX_PATH] = "";         // source folder containing audio files
    char dest_dir[_MAX_PATH] = "";        // destination folder (music library)
    char** fileList = NULL;               // list of files
    int fcount = 0;                       // number of files
    int successCount = 0;                 // number of files successfully processed

    // Read configuration file / initial setup
    if (setup(src_dir, dest_dir) != 0) {
        handle_error("Setup failed!\n");
        return 1;
    }

    // Get a list of the file names in 'src_dir' with extension 'ftype'
    fileList = get_filenames(src_dir, &fcount/*, ftype*/);

    printf("File List:\n");
    print_filenames(fileList, fcount);
    printf("\nResults:\n");

    // Read metadata and process files
    for (int i = 0; i < fcount; i++) {
        process_file(fileList[i], dest_dir, &successCount);
    }

    free(fileList);

    // Display summary
    print_summary(successCount, fcount);

    return 0;
}

// Function to process each file
void
process_file(filename, dest_dir, successCount)
    char* filename;
    const char* dest_dir;
    int* successCount;
{
    audioMetaData* meta = NULL;
    char oldPath[_MAX_PATH] = "";
    char newPath[_MAX_PATH] = "";
    bool mkdir_success = false;

    const char* ftype = get_file_extension(filename);

    if (!strcmp(ftype, "flac")) {
        meta = get_audioMetaData_flac(filename);
    } else if (!strcmp(ftype, "mp3")) {
        handle_error("Error: mp3 not yet implemented. ");
        // meta = get_audioMetaData_mp3(filename);
    } else {
        fprintf(stderr, "Error: Unsupported file type *.%s. ", ftype);
    }

    // procedure if meta contains metadata
    if (meta != NULL) {
        // copy the old pathname from the struct
        strcpy(oldPath, meta->pathname);

        // meta->pathname is modified by create_folder_structure()
        mkdir_success = create_folder_structure(meta, dest_dir);
    }

    // skip if a file contains no metadata or folder creation fails
    if (meta == NULL || !mkdir_success) {
        printf("Skipping [%s]\n", filename);
    } else {
        // copy the new pathname from the struct after modification
        strcpy(newPath, meta->pathname);

        if (rename(oldPath, newPath) == -1) {
            printf("%s ", filename);
            perror("could not be renamed");
        } else {
            // count and print files that did not fail
            printf("%s processed successfully.\n", newPath);
            (*successCount)++;
        }
    }

    free(meta);
    free(filename);
}

// Function to print the summary
void
print_summary(successCount, totalFiles)
    int successCount;
    int totalFiles;
{
    printf("%d files processed successfully,", successCount);
    printf(" %d files failed.\n\n", totalFiles - successCount);
}
