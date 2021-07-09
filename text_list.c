
#include "text_list.h"

int compare_items(const void *first, const void *second)
{
    const char **tmp_first = (const char **) first;
    const char **tmp_second = (const char **) second;
    return strcmp(*tmp_first, *tmp_second);
}

void sort_list(list *items_list)
{
    qsort(items_list->array, items_list->length, sizeof(char *), &compare_items);
}

list *create_list()
{
    list *new_list = malloc(sizeof(list));
    if (new_list == NULL) {
        fprintf(stderr, "[ERROR] Allocation has failed.\n");
        return NULL;
    }
    new_list->array = NULL;
    new_list->length = 0;
    new_list-> alocated_length = 0;
    return new_list;
}

void delete_list(list *items_list)
{
    for (size_t i = 0; i < items_list->length; i++) {
        free(items_list->array[i]);
    }
    free(items_list->array);
    free(items_list);
}

bool add_to_list(list *items_list, char *item)
{
    size_t length = strlen(item);
    char *tmp_item = malloc(sizeof(char) * (length + 1));
    if (tmp_item == NULL) {
        return fprintf(stderr, "[ERROR] Allocation failed.\n"), false;
    }
    strncpy(tmp_item, item, length + 1);

    if (items_list->alocated_length <= items_list->length) {
        items_list->alocated_length += 10;
        char **tmp = realloc(items_list->array, sizeof(char*) * items_list->alocated_length);
        if (tmp == NULL) {
            free(tmp_item);
            delete_list(items_list);
            fprintf(stderr, "[ERROR] Allocation failed.\n");
            return false; 
        }
        items_list->array = tmp;
    }    
    
    items_list->array[items_list->length] = tmp_item;
    items_list->length++;
    return true;
}
