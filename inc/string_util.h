#ifndef __STRING_UTIL_H_
#define __STRING_UTIL_H_ 1

#include <stdint.h>
#include <stddef.h>

#define STRING_LIST_BUFFER_SIZE 0x10

typedef struct
{
    size_t count;
    size_t buffer_size;

    const char** strings;
} string_list_t;

/**
 * @brief Initialize a string list
*/
void string_list_init(string_list_t* list);

/**
 * @brief Get the index of a string inside the list
*/
int string_list_get_index(string_list_t* list, const char* string);

/**
 * @brief Add an item to the string list
*/
void string_list_add(string_list_t* list, const char* string);

/**
 * @brief Remove the last item from the string list
*/
void string_list_remove_last(string_list_t* list);

/**
 * @brief Dispose a string list
*/
void string_list_dispose(string_list_t* list);

/**
 * @brief Like printf but for building a string together
 * 
 * @param[in] formatString string which has format information
 * @param[in] ... any other arguments
 * 
 * @returns the new formatted string
*/
const char* stringf(const char* formatString, ...);

/**
 * @brief Get the position of `c` inside `str`
 * 
 * @param str The string
 * @param c The delimiter
 * @returns The position of `c` if found, otherwise -1
 */
int strpos(const char* str, char c);

/**
 * @brief Get the last position of `c` inside `str`
 * 
 * @param str The string
 * @param c The delimiter
 * @returns The last position of `c` if found, otherwise the first position or -1 if not found
 */
int strlpos(const char* str, char c);

#endif
