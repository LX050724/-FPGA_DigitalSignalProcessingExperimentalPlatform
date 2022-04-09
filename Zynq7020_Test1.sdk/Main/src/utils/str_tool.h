//
// Created by yao on 2022/2/26.
//

#ifndef TELEPHONEDIRECTORY_STR_TOOL_H
#define TELEPHONEDIRECTORY_STR_TOOL_H

#include <stdbool.h>
#include "Array.h"

typedef struct {
    char *parent_str;
    char **list;
    int len;
} StringList;

char *str_malloc_copy(const char *str);
char *str_malloc_cat(const char *str1, const char *str2, char separator);
int isNumber(char c);
int isLetter(char c);
char *parse_string(char **input, int *len);

bool char_in_str(char c, char *str);
StringList str_split(const char *str, char *separator);
char *str_join(const StringList *str_list, int num, char separator);
void StringList_free(StringList *p);

#endif //TELEPHONEDIRECTORY_STR_TOOL_H
