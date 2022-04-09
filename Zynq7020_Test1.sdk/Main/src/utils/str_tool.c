//
// Created by yao on 2022/2/26.
//

#include "str_tool.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char *str_malloc_copy(const char *str) {
    if (str == NULL) return NULL;
    char *p = os_malloc(strlen(str) + 1);
    if (p) strcpy(p, str);
    return p;
}

char *str_malloc_cat(const char *str1, const char *str2, char separator) {
    char *p = os_malloc(strlen(str1) + strlen(str2) + 2);
    int write_index = 0;
    while (*str1) p[write_index++] = *str1++;
    if (separator) p[write_index++] = separator;
    while (*str2) p[write_index++] = *str2++;
    p[write_index] = 0;
    return p;
}

int isNumber(char c) {
    return (c <= '9' && c >= '0');
}

int isLetter(char c) {
    return ((c <= 'Z' && c >= 'A') ||
            (c <= 'z' && c >= 'a'));
}

char *parse_string(char **input, int *len) {
    if (input == NULL || *input == NULL || **input != '"') return NULL;
    char *str = *input + 1;
    char *start = str;
    int index = 0;

    while (*str != 0) {
        if (*str == '\\') {
            switch (str[1]) {
                case 'b':
                    start[index++] = '\b';
                    break;
                case 'f':
                    start[index++] = '\f';
                    break;
                case 'n':
                    start[index++] = '\n';
                    break;
                case 'r':
                    start[index++] = '\r';
                    break;
                case 't':
                    start[index++] = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    start[index++] = str[1];
                    break;
                default:
                    return NULL;
            }
            str += 2;
        } else if (*str == '"') {
            if (len) *len = index;
            *input = str + 1;
            return start;
        } else {
            start[index++] = *str++;
        }
    }
    return NULL;
}

bool char_in_str(char c, char *str) {
    while (*str) {
        if (c == *str) return true;
        else str++;
    }
    return false;
}

StringList str_split(const char *str, char *separator) {
    StringList str_list = {0};
    str_list.parent_str = str_malloc_copy(str);
    if (str_list.parent_str == NULL) return str_list;

    int start_index = 0;
    for (int i = 0; str_list.parent_str[i] != '\0'; i++) {
        if (char_in_str(str_list.parent_str[i], separator)) {
            if (start_index == i) {
                start_index++;
            } else {
            	str_list.parent_str[i] = 0;
                str_list.list = os_realloc(str_list.list, sizeof(char *) * (str_list.len + 1));
                str_list.list[str_list.len++] = str_list.parent_str + start_index;
                start_index = i + 1;
            }
        }
    }
    str_list.list = os_realloc(str_list.list, sizeof(char *) * (str_list.len + 1));
    str_list.list[str_list.len++] = str_list.parent_str + start_index;
    return str_list;
}

char *str_join(const StringList *str_list, int num, char separator) {
    if (str_list == NULL)return NULL;

    int total_len = 1;
    for (int i = 0; i < str_list->len && i < num; i++)
        total_len += strlen(str_list->list[i]);
    if (separator) total_len += str_list->len - 1;

    char *str = os_malloc(total_len);
    if (str == NULL) return NULL;

    int write_index = 0;
    for (int i = 0; i < str_list->len && i < num; i++) {
        char *str_slice = str_list->list[i];
        while (*str_slice) str[write_index++] = *str_slice++;
        if (separator) str[write_index++] = separator;
    }

    if (separator) str[write_index - 1] = 0;
    else str[write_index] = 0;
    return str;
}

void StringList_free(StringList *p) {
    if (p->parent_str) {
        os_free(p->parent_str);
    } else {
        for (int i = 0; i < p->len; i++)
            os_free(p->list[i]);
    }
    os_free(p->list);
}
