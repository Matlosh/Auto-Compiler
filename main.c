/* Made by: Matlosh | 2022 */

#include <stdio.h>
#include <dirent.h>
#include <windows.h>
#include <sys/stat.h>
#include <math.h>

#define MAX_FILES 256
#define MAX_FOLDERS 256
#define MAX_FILEPATH 512
#define MAX_COMMAND_LENGTH 1024
#define MAX_FLAGS_NUM 3
#define MAX_FLAG_LENGTH 5
#define MAX_FLAG_VALUE_LENGTH 1024
#define DEFAULT_WATCH_INTERVAL_TIME 1000

#define INCLUDE_SUBFOLDERS_FLAG 0x01
#define CUSTOM_FLAGS_FLAG 0x02
#define CUSTOM_WATCH_INTERVAL_FLAG 0x04

struct file {
    char filename[FILENAME_MAX];
    time_t last_changed_time;
};

struct flag {
    char flag[MAX_FLAG_LENGTH], flag_value[MAX_FLAG_VALUE_LENGTH];
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

// Retrieves flag value (f.e. from -Ctest retrieves "test")
// and saves it in the passed "value"
void get_flag_value(char *full_flag, char *flag, char *value) {
    int value_pos = strlen(flag);
    // char *ptr = strtok(full_flag, flag);
    // if(ptr != NULL) strcpy(value, ptr);
    // else strcpy(value, "");
    if(strstr(full_flag, flag) - full_flag != 0) {
        strcpy(value, "");
        return;
    }

    char flag_value[MAX_FLAG_VALUE_LENGTH];
    int flag_value_pos = 0;
    for(int i = strlen(flag); i < strlen(full_flag); i++, flag_value_pos++) {
        flag_value[flag_value_pos] = full_flag[i];
    }
    flag_value[flag_value_pos] = '\0';
    strcpy(value, flag_value);
}

// Finds flag value in given command_flags array and
// copies it to the given "value" variable, else copies empty ""
void get_flag_value_from_struct(struct flag *command_flags[MAX_FLAGS_NUM], char *flag, char *value) {
    for(int i = 0; i < MAX_FLAGS_NUM; i++) {
        if(command_flags[i] == NULL) return;

        if(strcmp(command_flags[i]->flag, flag) == 0) {
            strcpy(value, command_flags[i]->flag_value);
            return;
        }
    }

    strcpy(value, "");
}

// One process of comparing two files with the given time space difference
// between checks
void check_for_changes(char filenames[MAX_FILES][FILENAME_MAX], char folders[MAX_FOLDERS][FILENAME_MAX],
    char *executable_name, int flags, char *custom_flags, int watch_interval_time) {
    char command_args[MAX_COMMAND_LENGTH];
    command_args[0] = '\0';

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

    Sleep(watch_interval_time);
    struct file *files_array_2[MAX_FILES];
    create_files_array(filenames, files_array_2);

    if(compare_files_arrays(files_array_1, files_array_2) == 1) {
        char command[MAX_COMMAND_LENGTH];
        sprintf(command, "gcc %s -o %s %s", command_args, executable_name, custom_flags);
        system(command);
        printf("> File changes detected! Compiling...\n");
    }

    clear_files_array(files_array_1);
    clear_files_array(files_array_2);
}

void project_mode(char *executable_name, int flags, struct flag *command_flags[MAX_FLAGS_NUM]) {
    char filenames[MAX_FILES][FILENAME_MAX];
    char folders[MAX_FOLDERS][FILENAME_MAX];

    char custom_flags[MAX_FLAG_VALUE_LENGTH] = "";
    if(flags & CUSTOM_FLAGS_FLAG) {
        get_flag_value_from_struct(command_flags, "-C", custom_flags);
    }

    int watch_interval_time = DEFAULT_WATCH_INTERVAL_TIME;
    if(flags & CUSTOM_WATCH_INTERVAL_FLAG) {
        char custom_interval_time[MAX_FLAG_VALUE_LENGTH] = "";
        get_flag_value_from_struct(command_flags, "-T", custom_interval_time);

        int value_max_length = (int) log10(INT_MAX);
        if(strlen(custom_interval_time) > value_max_length) {
            printf("Error: Given value for watch interval time '-T' is too big.\n");
            exit(EXIT_SUCCESS);
        }

        watch_interval_time = atoi(custom_interval_time);
    }

    while(1) {
        memset(filenames, 0, sizeof(filenames));
        memset(folders, 0, sizeof(folders));
        get_all_files(filenames, folders, ".");
        check_for_changes(filenames, folders, executable_name, flags, custom_flags, watch_interval_time);
    }
}

int main(int argc, char *argv[]) {
    char *executable_name = argv[1];
    struct flag *command_flags[MAX_FLAGS_NUM];
    
    int flags = 0x00;
    int command_flags_pos = 0;
    for(int i = 2; i < argc; i++, command_flags_pos++) {
        char *full_flag = argv[i];
        
        char flag_value[MAX_FLAG_VALUE_LENGTH], flag[MAX_FLAG_LENGTH];
        if(strcmp(full_flag, "-I") == 0) {
            flags |= INCLUDE_SUBFOLDERS_FLAG;
            strcpy(flag, "-I");
        }
        if(strstr(full_flag, "-C") - argv[i] == 0) {
            flags |= CUSTOM_FLAGS_FLAG;
            strcpy(flag, "-C");
        }
        if(strstr(full_flag, "-T") - argv[i] == 0) {
            flags |= CUSTOM_WATCH_INTERVAL_FLAG;
            strcpy(flag, "-T");
        }

        get_flag_value(full_flag, flag, flag_value);

        struct flag *command_flag = malloc(sizeof(struct flag));
        strcpy(command_flag->flag, flag);
        strcpy(command_flag->flag_value, flag_value);

        command_flags[command_flags_pos] = command_flag;
    }
    command_flags[command_flags_pos] = NULL;

    project_mode(executable_name, flags, command_flags);

    for(int i = 0; i < MAX_FLAGS_NUM; i++) {
        if(command_flags[i] == NULL) break;
        free(command_flags[i]);
    }
    return 0;
}