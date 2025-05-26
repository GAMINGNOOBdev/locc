#include <string_util.h>
#include <file_util.h>
#include <logging.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#ifdef _WIN32
#   include <windows.h>
#   include <stdlib.h>
#else
#   include <dirent.h>
#endif

#define MAX_OF(a, b) a > b ? a : b

const char* KNOWN_EXTENSIONS[] = {
    ".c",
    ".cpp",
    ".cs",
    ".css",
    ".java",
    ".js",
    ".ts",
    ".h",
    ".hpp",
    ".rs",
    ".json",
    ".asm",
    ".s",
    ".py",
    ".yaml",
    ".cfg",
    ".fs",
    ".glsl",
    ".hlsl",
    ".html",
    ".cmake",
    ".log",
    0
};

file_info_t file_util_get_file_info(const char* filename)
{    
    file_info_t fileinfo = {0,0,-1,NULL,1};
    FILE* file = fopen(filename, "rb");
    if (file == NULL)
        return fileinfo;

    fileinfo.extension = file_util_get_extension(filename);
    for (int i = 0; i < EXTENSION_LAST; i++)
    {
        if (strcmp(fileinfo.extension, KNOWN_EXTENSIONS[i]) == 0)
        {
            fileinfo.extension_index = i;
            break;
        }
    }

    char* line = NULL;
    size_t buffersize = 0;
    size_t linesize = 0;
    while ((linesize = getlineV2(&line, &buffersize, file)) != (size_t)EOF)
    {
        if (linesize == 0)
        {
            fileinfo.empty_lines++;
            continue;
        }
        if (line[linesize-1] == '\n')
        {
            line[linesize-1] = 0;
            linesize--;
        }
        if (linesize == 0)
        {
            fileinfo.empty_lines++;
            continue;
        }
        if (line[linesize-1] == '\r')
        {
            line[linesize-1] = 0;
            linesize--;
        }

        if (linesize == 0)
            fileinfo.empty_lines++;
        else
            fileinfo.non_empty_lines++;
    }
    if (line)
        free(line);

    fclose(file);
    return fileinfo;
}

size_t file_util_file_size(const char* filename)
{
    size_t result = 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
        return 0;
    
    fseek(file, 0, SEEK_END);
    result = ftell(file);
    rewind(file);

    fclose(file);

    return result;
}

void* file_util_file_contents(const char* filename)
{
    size_t size = 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
        return NULL;
    
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    void* result = malloc(size+1);
    memset(result, 0, size+1);
    fread(result, 1, size, file);

    fclose(file);

    return result;
}

void file_util_concat_path_vectors(string_list_t* output, string_list_t* input, const char* prefix)
{
    if (output == NULL || input == NULL || prefix == NULL)
        return;

    if (input->count == 0)
        return;

    for (size_t i = 0; i < input->count; i++)
    {
        const char* string = input->strings[i];
        const char* formatted = stringf("%s%s", prefix, string);
        string_list_add(output, formatted);
    }
}

string_list_t file_util_get_directory_contents(const char* tmppath, int mask)
{
    string_list_t result;
    string_list_init(&result);
    char* path = malloc(strlen(tmppath)+1);
    strcpy(path, tmppath);

    if (path[strlen(path)-1] == '/' || path[strlen(path)-1] == '\\')
        path[strlen(path)-1] = 0;

#ifdef _WIN32
    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    if ((hFind = FindFirstFileA(stringf("%s\\*.*", path), &fdFile)) == INVALID_HANDLE_VALUE)
    {
        free(path);
        return;
    }

    do
    {
        if (fdFile.cFileName[0] == '.')
            continue;
        const char* filename = (const char*)fdFile.cFileName;

        if (mask == FilterMaskAllFilesAndFolders)
        {
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                string_list_t subdir_result = file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, filename), mask);
                if (subResult.count > 0)
                    file_util_concat_path_vectors(&result, &subdir_result, stringf("%s/", filename));
                string_list_dispose(&subdir_result);
            }

            string_list_add(&result, filename);
            continue;
        }

        if (mask == FilterMaskFilesAndFolders)
        {
            string_list_add(&result, filename);
            continue;
        }

        if (mask == FilterMaskFiles && !(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            string_list_add(&result, filename);
            continue;
        }

        if (mask == FilterMaskFolders && fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            string_list_add(&result, filename);
            continue;
        }

        if (mask == FilterMaskAllFiles)
        {
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                string_list_t subdir_result = file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, filename), mask);
                if (subResult.count > 0)
                    file_util_concat_path_vectors(&result, &subdir_result, stringf("%s/", filename));
                string_list_dispose(&subdir_result);
            }
            else
                string_list_add(&result, filename);
        }

        if (mask == FilterMaskAllFolders && fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            string_list_t subdir_result = file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, filename), mask);
            if (subResult.count > 0)
                file_util_concat_path_vectors(&result, &subdir_result, stringf("%s/", filename));

            string_list_add(&result, filename);
            string_list_dispose(&subdir_result);
        }
    }
    while (FindNextFileA(hFind, &fdFile));

    FindClose(hFind);
#else
    DIR* directory = opendir(path);
    if (directory == NULL)
    {
        LOGERROR(stringf("could not find folder '%s'", path));
        string_list_dispose(&result);
        free(path);
        return result;
    }

    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        if (mask == FilterMaskAllFilesAndFolders)
        {
            if (entry->d_type == DT_DIR)
            {
                string_list_t subResult = file_util_get_directory_contents(stringf("%s/%s", path, entry->d_name), mask);
                if (subResult.count > 0)
                    file_util_concat_path_vectors(&result, &subResult, stringf("%s/", entry->d_name));
                
                string_list_dispose(&subResult);
            }

            string_list_add(&result, entry->d_name);
            continue;
        }

        if (mask == FilterMaskFilesAndFolders)
        {
            string_list_add(&result, entry->d_name);
            continue;
        }

        if (mask == FilterMaskFiles && entry->d_type != DT_DIR)
        {
            string_list_add(&result, entry->d_name);
            continue;
        }

        if (mask == FilterMaskFolders && entry->d_type == DT_DIR)
        {
            string_list_add(&result, entry->d_name);
            continue;
        }

        if (mask == FilterMaskAllFiles)
        {
            if (entry->d_type == DT_DIR)
            {
                string_list_t subResult = file_util_get_directory_contents(stringf("%s/%s", path, entry->d_name), mask);
                if (subResult.count > 0)
                    file_util_concat_path_vectors(&result, &subResult, stringf("%s/", entry->d_name));
                
                string_list_dispose(&subResult);
            }
            else
                string_list_add(&result, entry->d_name);
            
            continue;
        }

        if (mask == FilterMaskAllFolders)
        {
            if (entry->d_type == DT_DIR)
            {
                string_list_t subResult = file_util_get_directory_contents(stringf("%s/%s", path, entry->d_name), mask);
                if (subResult.count > 0)
                    file_util_concat_path_vectors(&result, &subResult, stringf("%s/", entry->d_name));

                string_list_add(&result, entry->d_name);
            }

            continue;
        }
    }

    closedir(directory);
#endif

    free(path);
    return result;
}

const char* file_util_get_file_name(const char* str)
{
    char delimiter0 = '/';
    char delimiter1 = '\\';

    if (strpos(str, delimiter0) == -1 && strpos(str, delimiter1) == -1)
        return str;

    int lastPathSeperator = MAX_OF((int64_t)strlpos(str, delimiter0), (int64_t)strlpos(str, delimiter1));

    if (lastPathSeperator == -1)
        return str;
    
    return (const char*)&str[lastPathSeperator+1];
}

const char* file_util_get_extension(const char* str)
{
    int lastDot = -1;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '.')
            lastDot = i;
    }

    if (lastDot == -1)
        return str;
    
    return (const char*)&str[lastDot];
}

size_t getdelimV2(char **buffer, size_t *buffersz, FILE *stream, char delim)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (buffer == NULL)
        return -1;
    if (stream == NULL)
        return -1;
    if (buffersz == NULL)
        return -1;

    bufptr = *buffer;
    size = *buffersz;

    c = fgetc(stream);
    if (c == EOF)
        return -1;

    if (bufptr == NULL)
    {
        bufptr = malloc(128);
        if (bufptr == NULL)
            return -1;

        size = 128;
    }
    p = bufptr;
    while(c != EOF)
    {
        if ((p - bufptr) > (size - 1))
        {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL)
                return -1;
        }

        *p++ = c;
        if (c == delim)
            break;

        c = fgetc(stream);
    }

    *p++ = '\0';
    *buffer = bufptr;
    *buffersz = size;

    return p - bufptr - 1;
}

size_t getlineV2(char **buffer, size_t *buffersz, FILE *stream)
{
    return getdelimV2(buffer, buffersz, stream, '\n');
}
