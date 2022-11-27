/* Made by: Matlosh | 2022 */

#include <stdio.h>
#include <dirent.h>
#include <windows.h>
#include <sys/stat.h>

#define MAX_FILES 256
#define MAX_FOLDERS 256
#define MAX_FILEPATH 512
#define MAX_COMMAND_LENGTH 1024

#define INCLUDE_SUBFOLDERS_FLAG 0x01

struct file {
    char filename[FILENAME_MAX];
    time_t last_changed_time;
};

// Gets substring starting from start_pos to the end of the string and
// writes it to the given substring
void substr(char *filename, int start_pos, char *substring) {
    int j = 0;
    for(int i = 0; i < strlen(filename); i++) {
        if(start_pos < i) substring[j++] = filename[i];
    }

    substring[j] = '\0';
}

// Appends filename to the filenames array
void append_to_filenames(char filenames[MAX_FILES][FILENAME_MAX], char *filename) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(strlen(filenames[i]) == 0) {
            strcpy(filenames[i], filename);
            break;
        }
    }
}

// Appends folder to the folders array
void append_to_folders(char folders[MAX_FOLDERS][FILENAME_MAX], char *folder_path) {
    for(int i = 0; i < MAX_FOLDERS; i++) {
        if(strlen(folders[i]) == 0) {
            strcpy(folders[i], folder_path);
            break;
        }
    }
}

// Returns:
//  - 0 -> given "filename" is of type "extension"
//  - 1 -> given "filename" is a folder
//  - 2 -> given "filename" isn't of type "extension"
int get_file_marking(char *filename, char *extension) {
    char *last_dot_pos = strrchr(filename, '.');
    if(last_dot_pos == NULL) return 1;

    if(last_dot_pos != NULL) {
        char file_extension[strlen(filename)];
        substr(filename, last_dot_pos - filename, file_extension);

        if(strcmp(file_extension, extension) == 0) return 0;
        else return 2;
    }
}

// Gets all files with .c extension and adds their filename to the filenames array
void get_all_files(char filenames[MAX_FILES][FILENAME_MAX], char folders[MAX_FOLDERS][FILENAME_MAX], char *path) {
    DIR *dir = opendir(path);

    struct dirent *filename;
    while((filename = readdir(dir)) && filename != NULL) {
        if(filename->d_namlen < 3) continue;
        int file_marking = get_file_marking(filename->d_name, "c");
        
        char filepath[MAX_FILEPATH];
        sprintf(filepath, "%s/%s", path, filename->d_name);

        if(file_marking == 0) append_to_filenames(filenames, filepath);

        if(file_marking == 1) {
            append_to_folders(folders, filepath);
            get_all_files(filenames, folders, filepath);
        }
    }

    closedir(dir);
}

// Returns num of files from filenames array
int get_num_of_files(char filenames[MAX_FILES][FILENAME_MAX]) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(strlen(filenames[i]) == 0) return i;
    }

    return MAX_FILES;
}

// Populates array with struct file
void create_files_array(char filenames[MAX_FILES][FILENAME_MAX], struct file *file_array_ptr[MAX_FILES]) {
    for(int i = 0; i < MAX_FILES; i++) {
        struct file *file;
        
        if(strlen(filenames[i]) == 0) {
            file = NULL;
            file_array_ptr[i] = file;
            continue;
        }

        file = malloc(sizeof(struct file));
        strcpy(file->filename, filenames[i]);
        
        struct stat file_stat;
        stat(filenames[i], &file_stat);
        file->last_changed_time = file_stat.st_mtime;

        file_array_ptr[i] = file;
    }
}

// Clears given files array
void clear_files_array(struct file *file_array_ptr[MAX_FILES]) {
    for(int i = 0; i < MAX_FILES; i++) {
        if(file_array_ptr[i] == NULL) break;        
        free(file_array_ptr[i]);
    }
}

// Returns:
//  - 0 - if arrays are identical
//  - 1 - if arrays vary
int compare_files_arrays(struct file *file_array_ptr_1[MAX_FILES], struct file *file_array_ptr_2[MAX_FILES]) {
    for(int i = 0; i < MAX_FILES; i++) {
        if((file_array_ptr_1[i] != NULL && file_array_ptr_1[i] == NULL)
            || (file_array_ptr_1[i] == NULL && file_array_ptr_2[i] != NULL)) return 1;
        if(file_array_ptr_1[i] == NULL && file_array_ptr_2[i] == NULL) return 0;

        if(strcmp(file_array_ptr_1[i]->filename, file_array_ptr_2[i]->filename) != 0
            || file_array_ptr_1[i]->last_changed_time != file_array_ptr_2[i]->last_changed_time) {
            return 1;
        }
    }
}

// One process of comparing two files with the given time space difference
// between checks
void check_for_changes(char filenames[MAX_FILES][FILENAME_MAX], char folders[MAX_FOLDERS][FILENAME_MAX],
    char *executable_name, int flags) {
    char command_args[MAX_COMMAND_LENGTH];
    command_args[0] = '\0';

    printf("%d\n", flags);
    if(flags & INCLUDE_SUBFOLDERS_FLAG) {
        for(int i = 0; i < MAX_FOLDERS; i++) {
            if(strlen(folders[i]) == 0) break;
            strcat(command_args, "-I");
            strcat(command_args, folders[i]);
            strcat(command_args, " ");
        }
    }

    for(int i = 0; i < MAX_FILES; i++) {
        if(strlen(filenames[i]) == 0) break;
        strcat(command_args, filenames[i]);
        strcat(command_args, " ");
    }

    struct file *files_array_1[MAX_FILES];
    create_files_array(filenames, files_array_1);

    Sleep(1000);
    struct file *files_array_2[MAX_FILES];
    create_files_array(filenames, files_array_2);

    if(compare_files_arrays(files_array_1, files_array_2) == 1) {
        char command[MAX_COMMAND_LENGTH];
        sprintf(command, "gcc %s -o %s", command_args, executable_name);
        system(command);
        printf("%s\n", command);
        printf("> File changes detected! Compiling...\n");
    }

    clear_files_array(files_array_1);
    clear_files_array(files_array_2);
}

void project_mode(char *executable_name, int flags) {
    char filenames[MAX_FILES][FILENAME_MAX];
    char folders[MAX_FOLDERS][FILENAME_MAX];
    while(1) {
        memset(filenames, 0, sizeof(filenames));
        memset(folders, 0, sizeof(folders));
        get_all_files(filenames, folders, ".");
        check_for_changes(filenames, folders, executable_name, flags);
    }
}

int main(int argc, char *argv[]) {
    char *executable_name = argv[1];
    
    int flags = 0x00;
    for(int i = 2; i < argc; i++) {
        char *flag = argv[i];
        
        if(strcmp(flag, "-i") == 0) flags |= INCLUDE_SUBFOLDERS_FLAG;
    }


    project_mode(executable_name, flags);
    return 0;
}