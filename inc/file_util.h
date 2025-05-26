#ifndef __FILE_UTIL_H_
#define __FILE_UTIL_H_ 1

#include <string_util.h>
#include <stddef.h>
#include <stdio.h>

#define FilterMaskFiles                 0x00 // Only files inside the given directory
#define FilterMaskAllFiles              0x01 // All files including files from subfolders
#define FilterMaskFolders               0x02 // Only folder inside the given directory
#define FilterMaskAllFolders            0x03 // All folders including folders from subdirectories
#define FilterMaskFilesAndFolders       0x04 // Only files and folders inside the given directory
#define FilterMaskAllFilesAndFolders    0x05 // All files and folders including those in subdirectories

#define EXTENSION_LAST  22
extern const char* KNOWN_EXTENSIONS[];

typedef struct
{
    size_t empty_lines;
    size_t non_empty_lines;

    int extension_index;
    const char* extension;

    unsigned int file_count;
} file_info_t;

/**
 * @brief Gets file information
 * 
 * @param filename 
 * @return file_info_t 
 */
file_info_t file_util_get_file_info(const char* filename);

/**
 * Calculates the filesize of a given file
 * 
 * @param filename Path to the file
 * @returns Size of the file on the disk
*/
size_t file_util_file_size(const char* filename);

/**
 * Get the contents of a given file
 * 
 * @note Resulting pointer should be free-d by the user
 * 
 * @param filename Path to the file
 * @returns Contents of the file on the disk
*/
void* file_util_file_contents(const char* filename);

/**
 * Gets the contents of the given directory
 * @note This function may take a while to complete since it will retrieve all files from subfolders as well
 * 
 * @param path Path to the directory
 * @param mask A filter which decides how a directories' contents shall be gotten
 * 
 * @returns A list of files inside the given directory
*/
string_list_t file_util_get_directory_contents(const char* path, int mask);

/**
 * Gets the file name (with extension) without the leading path
 * @param filePath File path
 * @returns The file name without the prepending path
*/
const char* file_util_get_file_name(const char* filePath);

/**
 * Gets the file extension from a file path
 * @param filePath File path
 * @returns Only the extension from the file path (WITH a `.`)
*/
const char* file_util_get_extension(const char* filePath);

size_t getdelimV2(char **buffer, size_t *buffersz, FILE *stream, char delim);
size_t getlineV2(char **buffer, size_t *buffersz, FILE *stream);

#endif
