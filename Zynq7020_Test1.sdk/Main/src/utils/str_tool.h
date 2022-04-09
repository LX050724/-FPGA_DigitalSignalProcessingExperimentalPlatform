//
// Created by yao on 2022/2/26.
//

#ifndef TELEPHONEDIRECTORY_STR_TOOL_H
#define TELEPHONEDIRECTORY_STR_TOOL_H

char *str_malloc_copy(const char *str);
int isNumber(char c);
int isLetter(char c);
char *parse_string(char **input, int *len);

#endif //TELEPHONEDIRECTORY_STR_TOOL_H
