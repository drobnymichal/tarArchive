#ifndef TEXT_LIST_H
#define TEXT_LIST_H
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

typedef struct list {
    size_t alocated_length;
    size_t length;
    char **array;
} list;

void sort_list(list *items_list);
list *create_list();
void delete_list(list *items_list);
bool add_to_list(list *items_list, char *item);
int compare_items(const void *first, const void *second);


#endif