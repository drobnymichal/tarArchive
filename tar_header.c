#ifndef TAR_HEADER_C
#define TAR_HEADER_C
#include "tar_header.h"

void number_to_string(size_t full_size, int number, char *buffer)
{
    memset(buffer, 0, full_size);
    memset(buffer, '0', full_size - 1);
    size_t index = full_size - 2;
    while (number > 0) {
        buffer[index] = (number % 8) + '0';
        number /= 8;
        index--;
    }
}

bool check_sum(header *file_header)
{
    unsigned int check_sum = 0;
    unsigned char *data = (unsigned char *) file_header;
    
    for (size_t i = 0; i < 148 ; i++) {
        check_sum += data[i];
    }
    for (size_t i = 156; i < 512 ; i++) {
        check_sum += data[i];
    }
    check_sum += ' ' * 8;
    
    char *buffer = malloc(sizeof(char) * 7);
    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Allocation has failed\n");
        return false;
    }
    number_to_string(7, check_sum, buffer);
    //printf("check_sum: %s\n", buffer);
    //printf("check_sum: %s\n", file_header->check_sum);

    if (strncmp(file_header->check_sum, buffer, 7) != 0) {
        free(buffer);
        return false;
    }
    free(buffer);
    return true;
}

bool is_end(header *file_header)
{
    char *tmp = (char *) file_header;
    for (size_t i = 0; i < 512; i++) {
        if (tmp[i] != 0) {
            return false;
        }
    }
    //printf("end\n");
    return true;
}

header *make_empty_header()
{
    header *empty_header = malloc(sizeof(header));
    if (empty_header == NULL) {
        fprintf(stderr, "[ERROR] Allocation has failed.\n");
        return NULL;
    }
    memset(empty_header, 0, 512);
    return empty_header;
}

void print_header(header *file_header)
{
    printf("    filename: %s\n", file_header->file_name);
    printf("    mode:     %s\n", file_header->file_mod);
    printf("    owner_id: %s\n", file_header->uid);
    printf("    group_id: %s\n", file_header->gid);
    printf("    size:     %s\n", file_header->file_size);
    printf("    mtime:    %s\n", file_header->last_change);
    printf("    checksum: %s\n", file_header->check_sum);
    printf("    type:     %c\n", file_header->type_symptom[0]);
    printf("    link:     %s\n", file_header->link_name);
    printf("    magic:    %s\n", file_header->magic_number);
    printf("    UStar:    %c%c\n", file_header->tar_version[0], file_header->tar_version[1]);
    printf("    owner:    %s\n", file_header->owner_name);
    printf("    group:    %s\n", file_header->group_name);
    printf("    major:    %s\n", file_header->major_id);
    printf("    minor:    %s\n", file_header->minor_id);
    printf("    prefix:   %s\n\n", file_header->prefix);
}

void default_fill(header *file_header)
{
    file_header->tar_version[0] = '0';
    file_header->tar_version[1] = '0';
    strcpy(file_header->magic_number, "ustar");
    strcpy(file_header->major_id, "0000000");
    strcpy(file_header->minor_id, "0000000");
    memset(file_header->check_sum, ' ', sizeof(char) * 8);
}

void fill_check_sum(header *file_header, char *buffer)
{
    unsigned int check_sum = 0;
    unsigned char *data = (unsigned char *) file_header;
    
    for (size_t i = 0; i < sizeof(*file_header) ; i++) {
        check_sum += data[i];
    }
    number_to_string(7, check_sum, buffer);
    strcpy(file_header->check_sum, buffer);
}

bool fill_header_name(header *file_header, char *current_file)
{
    size_t position = 0;
    //printf("%lu\n", strlen(current_file));
    if (strlen(current_file) > 99) {
        position = strlen(current_file) - 99;
        //printf("%lu\n", position);
        for (size_t i = position; i < 99; i++) {
            if (current_file[i] == '/') {
                current_file[i] = 0;
                strcpy(file_header->prefix, current_file);
                stpcpy(file_header->file_name, current_file + i + 1);
                current_file[i] = '/';
                return true;
            }
        }
        fprintf(stderr, "[ERROR] Filepath is too long\n");
        return false;
    }
    strcpy(file_header->file_name, current_file);
    return true;
}

bool fill_header(header *file_header, struct stat *stats, char *current_file, int file_type)
{
    default_fill(file_header);
    
    if (!fill_header_name(file_header, current_file)) {
        return false;
    }

    char *buffer = malloc(sizeof(char) * 12);
    if (buffer == NULL) {
        fprintf(stderr, "[ERROR] Allocation has failed\n");
        return false;
    }
    if (file_type == 5) {
        number_to_string(12, 0, buffer);
    } else {
        number_to_string(12, stats->st_size, buffer);
    }
    strcpy(file_header->file_size, buffer);

    number_to_string(8, stats->st_mode & 511, buffer);
    strcpy(file_header->file_mod, buffer);
    //printf("%i\n", stats->st_mode);

    number_to_string(8, stats->st_uid, buffer);
    strcpy(file_header->uid, buffer);

    number_to_string(8, stats->st_gid, buffer);
    strcpy(file_header->gid, buffer);

    number_to_string(12, stats->st_mtime, buffer);
    strcpy(file_header->last_change, buffer);

    file_header->type_symptom[0] = file_type + '0';

    struct passwd *uid_stats = getpwuid(stats->st_uid);
    if (uid_stats != NULL) {
        strcpy(file_header->owner_name, uid_stats->pw_name);
    }

    struct group *gid_stats = getgrgid(stats->st_gid);
    if (gid_stats != NULL) {
        strcpy(file_header->group_name, gid_stats->gr_name);
    }

    fill_check_sum(file_header, buffer);

    free(buffer);

    //print_header(file_header);
    return true;
}

#endif