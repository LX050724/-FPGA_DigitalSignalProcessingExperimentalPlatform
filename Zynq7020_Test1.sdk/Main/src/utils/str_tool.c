//
// Created by yao on 2022/2/26.
//

#include "str_tool.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include <stdlib.h>
#include <string.h>

char *str_malloc_copy(const char *str) {
    char *p = os_malloc(strlen(str) + 1);
    if (p) strcpy(p, str);
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
