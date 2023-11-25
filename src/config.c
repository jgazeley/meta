#include "../include/config.h"

int is_valid_drive_path(const char* path) {
#ifdef _WIN32
    // Check for Windows drive letter and colon followed by optional backslash or forward slash
    return (isalpha(path[0]) && path[1] == ':' && (path[2] == '\0' || path[2] == '/' || path[2] == '\\'));
#else
    // Add Unix/Linux path validation if needed
    // For Unix/Linux, you might want to check for a leading slash '/'
    // For simplicity, this example assumes a relative path is valid
    return 1; // Always return true for non-Windows platforms in this example
#endif
}

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
int setup(char* src_path, char* dest_path)
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
            return 1;
        }

        // Initialize configuration file
        fprintf(cfg, "[Directory]\nSource=\nDestination=\n");
        printf("%s created successfully, please set the ", config);
        printf("source and destination path.\n");
        fclose(cfg);

        // Open configuration file in notepad and terminate this program
        sprintf(cmd, "notepad %s", config);
        system(cmd);
        return 1;
    }

    // Read source and destination path from the config file
    while (fgets(line, _MAX_PATH, cfg)) {
        if (!strncmp(line, "Source=", strlen("Source="))) {
            strcpy(src_path, strchr(line, '=') + 1);
            len = strlen(src_path);
            if (len > 0 && src_path[len - 1] == '\n')
                src_path[len - 1] = '\0';            
        
            // Check if dest_dir is a valid drive path
            if (!is_valid_drive_path(src_path)) {
                fprintf(stderr, "Error (dir.ini): The source directory '%s' is not a valid drive path.\n", src_path);
                return 1;
            }

            // Check if dest_dir is a valid directory with read and write permissions
            if (_access(src_path, 4) != 0 || _access(src_path, 2) != 0) {
                fprintf(stderr, "Error (dir.ini): The source directory '%s' is not a valid directory or does not have read and write permissions.\n", src_path);
                return 1;
            }
        }

        if (!strncmp(line, "Destination=", strlen("Destination="))) {
            strcpy(dest_path, strchr(line, '=') + 1);
            len = strlen(dest_path);
            if (len > 0 && dest_path[len - 1] == '\n')
                dest_path[len - 1] = '\0';

            // Check if dest_dir is a valid drive path
            if (!is_valid_drive_path(dest_path)) {
                fprintf(stderr, "Error (dir.ini): The destination directory '%s' is not a valid drive path.\n", dest_path);
                return 1;
            }

            // Check if dest_dir is a valid directory with read and write permissions
            if (_access(dest_path, 4) != 0 || _access(dest_path, 2) != 0) {
                fprintf(stderr, "Error (dir.ini): The destination directory '%s' is not a valid directory or does not have read and write permissions.\n", dest_path);
                return 1;
            }
        }
    } fclose(cfg);
    return 0;
}

// Test
// int main()
// {
//     char src[_MAX_PATH] = "";
//     char dest[_MAX_PATH] = "";
//     setup(src, dest);
//     printf("Source:        %s\n", src);
//     printf("Destination:   %s\n", dest);
// }