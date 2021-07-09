#ifndef TAR_HEADER_H
#define TAR_HEADER_H
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct header {
    char file_name[100];
    char file_mod[8];
    char uid[8];
    char gid[8];
    char file_size[12];
    char last_change[12];
    char check_sum[8];
    char type_symptom[1];
    char link_name[100];
    char magic_number[6];
    char tar_version[2];
    char owner_name[32];
    char group_name[32];
    char major_id[8];
    char minor_id[8];
    char prefix[155];
    char empty[12];
} header;

bool check_sum(header *file_header);
bool is_end(header *file_header);
header *make_empty_header();
void print_header(header *file_header);
bool fill_header(header *file_header, struct stat *stats, char *current_file, int file_type);
void number_to_string(size_t full_size, int number, char *buffer);

#endif