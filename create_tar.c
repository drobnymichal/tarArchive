
#include "create_tar.h"

const int DIRECTORY = 5;
const int REG_FILE = 0; 

bool write_into_tar(char *path_to_file, int tar_fd, struct stat *stats)
{
    int fd = open(path_to_file, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "[ERROR] Problem with openning file\n");
        return false;
    }
        
    char *content = malloc(sizeof(char) * 512);
    if (content == NULL) {
        close(fd);
        fprintf(stderr, "[ERROR] Allocation of memory has failed.\n");
        return false;
    }
    
    int iterations = (stats->st_size / 512);
    if ((stats->st_size % 512) > 0) {
        iterations++;
    }
    for (int i = 0; i < iterations; i++) {
        ssize_t bytes = read(fd, content, 512);
        memset(content + bytes, 0, sizeof(char) * (512 - bytes));
        if (write(tar_fd, content, 512) < 512) {
            close(fd);
            fprintf(stderr, "[ERROR] Cannot write into tar file\n");
            free(content);
            return false;
        }
    }
    close(fd);
    free(content);
    return true;
}

char *make_actual_name(char *prefix, char *sufix)
{
    char *full_path = malloc(sizeof(char) * (strlen(prefix) + strlen(sufix) + 2));
    if (full_path == NULL) {
        fprintf(stderr, "[ERROR] Allocation has failed\n.");
        return NULL;
    }
    strcpy(full_path, prefix);
    strcat(full_path, sufix);
    return full_path;
}

void write_dir_content(char *actual_path, setup *options, list *files_names)
{
    DIR *dir = NULL;
    if ((dir = opendir(actual_path)) == NULL) {
        fprintf(stderr, "[ERROR] Cannot open directory: %s\n", actual_path);
        options->return_value = EXIT_FAILURE;
        return;
    }

    struct dirent *dire_entry = NULL;
    while ((dire_entry = readdir(dir)) != NULL) {
        if (strcmp(".", dire_entry->d_name) == 0 
        || strcmp("..", dire_entry->d_name) == 0
        || strcmp(options->tar_name, actual_path) == 0) {
            continue;
        }
        add_to_list(files_names, dire_entry->d_name);
    }

    closedir(dir);
    return;
}

bool work_with_file(char *actual_path, setup *options, struct stat *stats)
{
    return write_into_tar(actual_path, options->tar_fd, stats);
}

bool work_with_dir(char *actual_path, setup *options)
{
    list *new_files_names = NULL;
    new_files_names = create_list();
    if (new_files_names == NULL) {
        return false;
    }
    write_dir_content(actual_path, options, new_files_names);

    return make_tar(options, new_files_names, actual_path);
}   

bool make_tar(setup *options, list *file_names, char *prefix)
{
    sort_list(file_names);
    for (size_t i = 0; i < file_names->length; i++) {
        char *actual_path = make_actual_name(prefix, file_names->array[i]);
        if (actual_path == NULL) {
            delete_list(file_names);
            return false;
        }

        struct stat stats;
        if (stat(actual_path, &stats) == -1) {
            options->return_value = EXIT_FAILURE;
            if (options->report_tar) {
                fprintf(stderr, "%s\n", actual_path);
            }
            fprintf(stderr, "[ERROR] Cannot check stats of file: %s\n", actual_path);
            free(actual_path);
            continue;
        }

        if (strlen(actual_path) > 254) {
            fprintf(stderr, "[ERROR] Too long path\n");
            free(actual_path);
            delete_list(file_names);
            return false;
        }

        if ((stats.st_mode & S_IFMT) != S_IFREG && (stats.st_mode & S_IFMT) != S_IFDIR) {
            free(actual_path);
            continue;
        }

        int file_type = REG_FILE;
        if ((stats.st_mode & S_IFMT) == S_IFDIR) {
            file_type = DIRECTORY;
            size_t length = strlen(actual_path);
            if (actual_path[length - 1] != '/') {
                actual_path[length] = '/';
                actual_path[length + 1] = 0;
            }
        }

        if (options->report_tar) {
            fprintf(stderr, "%s\n", actual_path);
        }
        if (strlen(actual_path) > 254) {
            fprintf(stderr, "[ERROR] Path too big: %s\n", actual_path);
        }
        header *tar = make_empty_header();
        if (tar == NULL) {
            free(actual_path);
            delete_list(file_names);
            return false;
        }
        if (!fill_header(tar, &stats, actual_path, file_type)) {
            free(tar);
            free(actual_path);
            delete_list(file_names);
            return false;
        }

        if (options->print_header) {
            print_header(tar);
        }

        ssize_t ret_val;
        if((ret_val = write(options->tar_fd, tar, sizeof(char) * 512)) == -1) {
            fprintf(stderr, "[ERROR] Cannot write into file.\n");
            free(tar);
            free(actual_path);
            delete_list(file_names);
            return false;
        }
        
        if ((stats.st_mode & S_IFMT) == S_IFREG) {
            if (!work_with_file(actual_path, options, &stats)) {
                free(tar);
                free(actual_path);
                delete_list(file_names);
                return false;
            }

        } else if ((stats.st_mode & S_IFMT) == S_IFDIR) {
            if (!work_with_dir(actual_path, options)) {
                free(tar);
                free(actual_path);
                delete_list(file_names);
                return false;
            }
        }
        free(tar);
        free(actual_path);
    }
    delete_list(file_names);
    return true;
}

