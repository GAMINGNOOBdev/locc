#include <argument_parser.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __APPLE__
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif

#ifdef _WIN32
#   include <windows.h>
#   include <stdlib.h>
#   define OS_DELIMITER '\\'
#else
#   include <dirent.h>
#   define OS_DELIMITER '/'
#endif

typedef struct
{
    size_t empty_lines;
    size_t non_empty_lines;
    int extension_index;
    const char* extension;
    size_t file_count;
} file_info_t;

typedef void(*file_iteration_callback_t)(const char* path, const char* filename);

#define PRINT_VERSION printf("LOCC version %d.%d.%d\n\n", __YEAR__, __MONTH__, __DAY__)

void print_help_message(void)
{
    printf("Usage:\n\tlocc [options] <path1> <path2> ...\nOptions:\n\t-h\t\t\tPrint this help message\n\t-i/--ignore-misc\tIgnore miscellaneous files\n");
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

#ifdef _WIN32
#define getline(buffer, buffersz, stream) getdelim(buffer, buffersz, stream, '\n')
size_t getdelim(char **buffer, size_t *buffersz, FILE *stream, char delim)
{
    char *bufptr = NULL;
    char *p = bufptr;
    int size;
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

        *p = c;
        p++;
        if (c == delim)
            break;

        c = fgetc(stream);
    }

    *p++ = '\0';
    *buffer = bufptr;
    *buffersz = size;

    return p - bufptr - 1;
}
#endif

#define EXTENSION_COUNT  24
const char* KNOWN_EXTENSIONS[] = {
    ".asm",
    ".c",
    ".cfg",
    ".cmake",
    ".cpp",
    ".cs",
    ".css",
    ".fs",
    ".glsl",
    ".h",
    ".hlsl",
    ".hpp",
    ".html",
    ".js",
    ".json",
    ".java",
    ".log",
    ".md",
    ".py",
    ".rs",
    ".s",
    ".ts",
    ".txt",
    ".yaml"
};

int ignore_misc = 0;
char* LINE_PTR = NULL;
size_t LINE_PTR_SIZE = 0;

file_info_t directory_information[EXTENSION_COUNT+1];
file_info_t total_directory_information;
file_info_t grand_total;
const char* current_path = NULL;

const char* file_util_get(const char* str, char delim)
{
    int lastDot = strlpos(str, delim);
    if (lastDot == -1)
        return str;

    return (const char*)&str[lastDot];
}

file_info_t get_file_info(const char* filename)
{
    file_info_t fileinfo = {0,0,-1,NULL,1};
    fileinfo.extension = file_util_get(filename, '.');
    const char* name = file_util_get(filename, OS_DELIMITER)+1;
    if (strcmp(name, "CMakeLists.txt") == 0)
        fileinfo.extension = KNOWN_EXTENSIONS[3];
    for (int i = 0; i < EXTENSION_COUNT; i++)
    {
        if (strcmp(fileinfo.extension, KNOWN_EXTENSIONS[i]) == 0)
        {
            fileinfo.extension = KNOWN_EXTENSIONS[i];
            fileinfo.extension_index = i;
            break;
        }
    }

    if (ignore_misc && fileinfo.extension_index == -1)
        return fileinfo;

    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return fileinfo;

    size_t linesize = 0;
    while ((linesize = getline(&LINE_PTR, &LINE_PTR_SIZE, file)) != (size_t)EOF)
    {
        char* line = LINE_PTR;
        if (linesize == 0)
        {
            fileinfo.empty_lines++;
            continue;
        }
        if (line[linesize-1] == '\n')
            linesize--;

        if (linesize == 0)
        {
            fileinfo.empty_lines++;
            continue;
        }
        if (line[linesize-1] == '\r')
            linesize--;

        if (linesize == 0)
            fileinfo.empty_lines++;
        else
            fileinfo.non_empty_lines++;
    }

    fclose(file);
    return fileinfo;
}

void file_iteration(const char* path, const char* filename)
{
    const char* finalpath = path;
    if (filename != NULL)
        finalpath = stringf("%s/%s", path, filename);
    file_info_t info = get_file_info(finalpath);
    if (info.extension_index == -1 && !ignore_misc)
        info.extension_index = EXTENSION_COUNT;

    directory_information[info.extension_index].extension = info.extension;
    directory_information[info.extension_index].empty_lines += info.empty_lines;
    directory_information[info.extension_index].non_empty_lines += info.non_empty_lines;
    directory_information[info.extension_index].file_count++;

    total_directory_information.empty_lines += info.empty_lines;
    total_directory_information.non_empty_lines += info.non_empty_lines;
    total_directory_information.file_count++;

    grand_total.empty_lines += info.empty_lines;
    grand_total.non_empty_lines += info.non_empty_lines;
    grand_total.file_count++;
}

void file_util_iterate_directory_all_files(const char* tmppath, file_iteration_callback_t callback)
{
    if (!tmppath || !callback)
        return;

    char* path = malloc(strlen(tmppath)+1);
    strcpy(path, tmppath);

    if (path[strlen(path)-1] == '/' || path[strlen(path)-1] == '\\')
        path[strlen(path)-1] = 0;

    // check if possibly the given path is just a single file
    FILE* file = fopen(path, "r+");
    if (file != NULL)
    {
        fclose(file);
        callback(path, NULL);
        return;
    }

#ifdef _WIN32
    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    if ((hFind = FindFirstFileA(stringf("%s\\*.*", path), &fdFile)) == INVALID_HANDLE_VALUE)
    {
        printf("ERROR: could not find folder '%s'\n", path);
        free(path);
        return;
    }

    do
    {
        if (fdFile.cFileName[0] == '.')
            continue;
        const char* filename = (const char*)fdFile.cFileName;

        if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            file_util_iterate_directory_all_files(stringf("%s/%s", path, filename), callback);
        else
            callback(path, filename);
    }
    while (FindNextFileA(hFind, &fdFile));

    FindClose(hFind);
#else
    DIR* directory = opendir(path);
    if (directory == NULL)
    {
        printf("ERROR: could not find folder '%s' \n", path);
        free(path);
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR)
            file_util_iterate_directory_all_files(stringf("%s/%s", path, entry->d_name), callback);
        else
            callback(path, entry->d_name);
    }

    closedir(directory);
#endif

    free(path);
}

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        print_help_message();
        return 0;
    }

    argument_parser parser;
    argument_parser_init(&parser);
    argument_parser_add(&parser, Argument_Type_Flag, "-h", "--help");
    argument_parser_add(&parser, Argument_Type_Flag, "-i", "--ignore-misc");
    argument_parser_parse(&parser, argc, argv);

    if (argument_parser_has(&parser, "-h"))
    {
        print_help_message();
        argument_parser_dispose(&parser);
        return 0;
    }

    ignore_misc = argument_parser_has(&parser, "-i");

    PRINT_VERSION;

    LINE_PTR = malloc(0x1000);
    LINE_PTR_SIZE = 0x1000;

    memset(&grand_total, 0, sizeof(file_info_t));
    for (size_t i = 0; i < parser.unparsed.size; i++)
    {
        memset(directory_information, 0, sizeof(directory_information));
        memset(&total_directory_information, 0, sizeof(file_info_t));

        const char* filePath = parser.unparsed.data[i].values[0];
        if (filePath[strlen(filePath)-1] == '/' || filePath[strlen(filePath)-1] == '\\')
            ((char*)filePath)[strlen(filePath)-1] = 0;

        current_path = filePath;
        file_util_iterate_directory_all_files(filePath, file_iteration);

        if (total_directory_information.file_count == 0)
            continue;

        printf("Info for '%s'\n===========", filePath);
        for (size_t i = 0; i < strlen(filePath); i++)
            printf("=");
        printf("\n");
        printf("\ttotal:\t%ld files | lines: %ld code | %ld empty | (%ld total)\n", total_directory_information.file_count, total_directory_information.non_empty_lines, total_directory_information.empty_lines, total_directory_information.non_empty_lines + total_directory_information.empty_lines);
        for (int i = 0; i < EXTENSION_COUNT; i++)
        {
            file_info_t info = directory_information[i];
            if (info.file_count == 0)
                continue;
            printf("\t%s:\t%ld files | lines: %ld code | %ld empty | (%ld total)\n", info.extension, info.file_count, info.non_empty_lines, info.empty_lines, info.non_empty_lines + info.empty_lines);
        }
        file_info_t info = directory_information[EXTENSION_COUNT];
        if (info.file_count > 0)
            printf("\tmisc:\t%ld files | lines: %ld code | %ld empty | (%ld total)\n", info.file_count, info.non_empty_lines, info.empty_lines, info.non_empty_lines + info.empty_lines);
        printf("\n");
    }

    if (parser.unparsed.size > 1)
    {
        printf("Info Summary\n============\n");
        printf("\t%ld files\n\t%ld lines of code\n\t%ld empty lines\n\t(%ld total lines)\n\n", grand_total.file_count, grand_total.non_empty_lines, grand_total.empty_lines, grand_total.non_empty_lines + grand_total.empty_lines);
    }

    free(LINE_PTR);

    argument_parser_dispose(&parser);

    return 0;
}
