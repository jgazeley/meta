#include "../include/metadata.h"

int main(int argc, char *argv[])
{
    char* program_name = argv[0];
    char* artist = NULL;

    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <folder_name>\n", program_name);
        return 1;
    }

    artist = argv[1];

    if (create_artist_folder(artist))
        printf("Folder created successfully!\n");
    else
        printf("Folder creation failed!!\n");

    return 0;
}
