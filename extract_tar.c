#ifndef EXTRACT_TAR_C
#define EXTRACT_TAR_C
#include "extract_tar.h"

int poww(int num, int power)
{
    int result = 1;
    for (int i = 0; i < power; i++) {
        result *= num;
    }
    return result;
}

bool file_exists(char *path)
{
    struct stat st;
    if (stat(path, &st) == -1) {
        return false;
    }
    return true;
}

int to_decimal(int octal_num) {
    long int result = 0;

    for (int i = 0; octal_num > 0; i++) {
        result += (octal_num % 10) * poww(8, i);
        octal_num /= 10;
    }

    return result;
}

bool write_into_file(long int size, int file_fd, int tar_fd)
{
    int iterations = size / 512;
    if ((size % 512) > 0) {
        iterations++;
    }
    for (int i = 0; i < iterations; i++) {
        char buffer[512];
        ssize_t ret_val = read(tar_fd, buffer, 512);
        if (ret_val <= 0) {
            perror("[ERROR] Resulting tar file");
            return false;
        }
    
        if (ret_val < 512) {
            fprintf(stderr, "[ERROR] Resulting tar file has bad format\n");
            return false;
        }
        long int bytes_to_write = 512;
        if (size < 512) {
            bytes_to_write = size;
        }
        if (write(file_fd, buffer, bytes_to_write) < bytes_to_write) {
            fprintf(stderr, "[ERROR] Problem with writting content into resulting file\n");
            return false;
        }
        size -= 512;
    }
    return true;
}

void make_path(char *result, char *prefix, char *sufix)
{
    result[0] = 0;
    if (strlen(prefix) > 0) {
        strcpy(result, prefix);
        strcat(result, "/");
    }
    strcat(result, sufix);
}

bool recursive_mkdir(char *path)
{
    size_t last_slash = 0;
    size_t length = strlen(path);
    size_t stop = length;

    if (path[length - 1] == '/') {
        stop--;
    }

    for (size_t i = 0; i < stop; i++) {
        if (path[i] == '/') {
            last_slash = i;
        }
    }
    for (size_t i = 0; i < last_slash + 1; i++) {
        if (path[i] == '/') {
            path[i] = 0;
            if (!file_exists(path)) {
                if (mkdir(path, 0777) == -1 && errno != EEXIST) { 
                    perror("[ERROR] mkdir():");
                    path[i] = '/';
                    return false;
                }
            }
            path[i] = '/';
        }
    }
    return true;
}

bool create_file(header *file_head, int tar_fd, bool report)
{
    char name[255];
    name[0] = 0;
    make_path(name, file_head->prefix, file_head->file_name);
    int file_fd;
    if (report) {
        fprintf(stderr, "%s\n", name);
    }

    if (file_exists(name)) {
        fprintf(stderr, "[ERROR] File: %s already exists\n", name);
        return false;
    }

    if (!recursive_mkdir(name)) {
        return false;
    }

    char *endptr;
    long int mod = strtoll(file_head->file_mod, &endptr, 8);

    if ((file_fd = open(name, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, mod)) == -1) {
        perror("[ERROR] Extracting archive: creat():");
        return false;
    }

    long int size = strtoll(file_head->file_size, &endptr, 8);

    if (!write_into_file(size, file_fd, tar_fd)) {
        close(file_fd);
        return false;
    }
    close(file_fd);
    long int time_in_sec = strtoll(file_head->last_change, &endptr, 8);

    struct utimbuf times;
    times.modtime = time_in_sec;
    times.actime = time_in_sec;

    if (utime(name, &times) == -1) {
        perror("[ERROR] utime():");
        return false;
    }
    return true;
}

bool create_directory(header *file_head, bool report)
{
    char name[255];
    name[0] = 0;
    make_path(name, file_head->prefix, file_head->file_name);
    if (report) {
        fprintf(stderr, "%s\n", name);
    }

    if (!recursive_mkdir(name)) {
        return false;
    }

    if (file_exists(name)) {
        return true;
    }

    if ((mkdir(name, to_decimal(atoi(file_head->file_mod)))) == -1 && errno != EEXIST) {
        fprintf(stderr, "[ERROR] Cannot create directory %s\n", file_head->file_name);
        return false;
    }

    char *endptr;
    long int time_in_sec = strtoll(file_head->last_change, &endptr, 8);

    struct utimbuf times;
    times.modtime = time_in_sec;
    times.actime = time_in_sec;
    if (utime(name, &times) == -1) {
        perror("[ERROR] utime():");
        return false;
    }
    return true;
}

bool extract_tar(int tar_fd, bool report)
{
    header *file_head = make_empty_header();
    if (file_head == NULL) {
        return false;
    }

    ssize_t bytes = read(tar_fd, file_head, 512);
    if (bytes < 512) {
        free(file_head);
        fprintf(stderr, "[ERROR] File has not tar format.\n");
        return false;
    }

    while (!is_end(file_head)) {
        if (!check_sum(file_head)) {
            free(file_head);
            fprintf(stderr, "[ERROR] Check sum does not fit to the header.\n");
            return false;
        }
        
        if (file_head->type_symptom[0] == '0') {
            if (!create_file(file_head, tar_fd, report)) {
                free(file_head);
                return false;
            }
        } else if (file_head->type_symptom[0] == '5') {
            if (!create_directory(file_head, report)) {
                free(file_head);
                return false;
            }
        }
        ssize_t bytes = read(tar_fd, file_head, 512);
        if (bytes < 512) {
            free(file_head);
            fprintf(stderr, "[ERROR] File has not tar format.\n");
            return false;
        }
    }
    free(file_head);
    return true;
}

#endif