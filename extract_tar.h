#ifndef EXTRACT_TAR_H
#define EXTRACT_TAR_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include "tar_header.h"
#include <utime.h>
#include <time.h>

bool extract_tar(int tar_fd, bool report);
bool create_directory(header *file_head, bool report);
bool create_file(header *file_head, int tar_fd, bool report);
bool write_into_file(long int size, int file_fd, int tar_fd);
int to_decimal(int octal_num);
int poww(int num, int power);


#endif