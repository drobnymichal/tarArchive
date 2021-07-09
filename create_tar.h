#ifndef CREATE_TAR_H
#define CREATE_TAR_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include "tar_header.h"
#include "text_list.h"
#include "tar_options.h"



bool make_tar(setup *options, list *file_names, char *prefix);
bool write_into_tar(char *path_to_file, int tar_fd, struct stat *stats);
char *make_actual_name(char *prefix, char *sufix);
void write_dir_content(char *actual_path, setup *options, list *files_names);



#endif