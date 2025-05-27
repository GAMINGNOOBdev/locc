#include <string_util.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

void string_list_init(string_list_t* list)
{
    if (list == NULL)
        return;

    list->count = 0;
    list->buffer_size = 0;
    list->strings = NULL;
}

int string_list_get_index(string_list_t* list, const char* string)
{
    if (list == NULL)
        return -1;

    for (size_t i = 0; i < list->count; i++)
        if (strcmp(list->strings[i], string) == 0)
            return i;

    return -1;
}

void string_list_add(string_list_t* list, const char* string)
{
    if (list == NULL || string == NULL)
        return;

    if (string_list_get_index(list, string) != -1)
        return;

    if (list->count + 1 >= list->buffer_size)
    {
        list->buffer_size += STRING_LIST_BUFFER_SIZE;
        list->strings = realloc(list->strings, sizeof(const char*) * list->buffer_size);
    }

    list->strings[list->count] = malloc(strlen(string)+1);
    strcpy((char*)list->strings[list->count], string);
    list->count++;
}

void string_list_remove_last(string_list_t* list)
{
    if (list == NULL)
        return;

    if (list->count == 0)
        return;

    free((void*)list->strings[list->count-1]);
    list->count--;
}

void string_list_dispose(string_list_t* list)
{
    if (list == NULL || list->strings == NULL)
        return;

    for (size_t i = 0; i < list->count; i++)
    {
        if (list->strings[i] == NULL)
            continue;

        free((void*)list->strings[i]);
    }
    free(list->strings);

    list->count = 0;
    list->buffer_size = 0;
    list->strings = NULL;
}

const char* stringf(const char* formatString, ...)
{
    static char mFormattingBuffer[4096];

    va_list args;
    va_start(args, formatString);
    vsnprintf(mFormattingBuffer, 4096, formatString, args);
    va_end(args);

    return mFormattingBuffer;
}

int strpos(const char* str, char c)
{
    char* s = (char*)str;
    int idx = 0;
    while (*s != 0)
    {
        if (*s == c)
            return idx;
        s++;
        idx++;
    }
    return -1;
}

int strlpos(const char* str, char c)
{
    char* s = (char*)str;
    int idx = 0;
    int resIdx = -1;
    while (*s != 0)
    {
        if (*s == c)
            resIdx = idx;

        s++;
        idx++;
    }
    return resIdx;
}
