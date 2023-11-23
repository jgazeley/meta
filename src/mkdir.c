#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_LENGTH 64
#define FULL_PERMISSIONS 0777

int create_artist_folder(char* artist)
{
    char folder_name[MAX_LENGTH] = "";
    char* artistNoTHE = NULL;

    // If the artist name starts with "The ", move The to the end. (The Band -> Band, The)
    if (strncmp(artist, "The ", strlen("The ")) == 0) {
        artistNoTHE = strchr(artist, ' ') + 1;
        int result = snprintf(folder_name, MAX_LENGTH, "%s, The", artistNoTHE);

        if (result < 0 || result >= MAX_LENGTH) {
            fprintf(stderr, "Error formatting folder name.\n");
            return 0;
        }
    } else {
        // If the artist name doesn't start with "The ", just copy the artist name
        int result = snprintf(folder_name, MAX_LENGTH, "%s", artist);

        if (result < 0 || result >= MAX_LENGTH) {
            fprintf(stderr, "Error formatting folder name.\n");
            return 0;
        }
    }

    // Use the mkdir function to create a new directory
    if (mkdir(folder_name, FULL_PERMISSIONS) == 0) {
        printf("Directory '%s' created successfully.\n", folder_name);
        return 1;
    } else {
        perror("Error creating directory");
        return 0;
    }
}

int main(int argc, char *argv[])
{
    char* program_name = argv[0];
    char* artist = NULL;

    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <folder_name>\n", program_name);
        return EXIT_FAILURE;
    }

    artist = argv[1];

    if(create_artist_folder(artist))
        printf("Folder created successfully!\n");
    else
        printf("Folder creation failed!!\n");

    return 0;
}
