#ifndef TAR_OPTIONS_H
#define TAR_OPTIONS_H


typedef struct setup {
    bool create_tar;
    bool extract_tar;
    bool report_tar;
    bool print_header;
    int return_value;
    char *tar_name;
    int tar_fd;
    int files_in_archive;
    list *files_names;
} setup;

#endif