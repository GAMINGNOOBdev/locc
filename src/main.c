#include <argument_parser.h>
#include <string_util.h>
#include <file_util.h>
#include <logging.h>
#include <string.h>
#include <stdio.h>

#ifdef __APPLE__
#   include <stdlib.h>
#else
#   include <malloc.h>
#endif

#define PRINT_VERSION printf("LOCC version %d.%d.%d\n\n", __YEAR__, __MONTH__, __DAY__)

void print_help_message(void)
{
    printf("Usage:\n\tlocc [options] <project paths...>\nOptions:\n\t-h\tPrint this help message\n");
}

char* LINE_PTR = NULL;
size_t LINE_PTR_SIZE = 0;

file_info_t get_file_info(const char* filename)
{
    file_info_t fileinfo = {0,0,-1,NULL,1};
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return fileinfo;

    fileinfo.extension = file_util_get_extension(filename);
    for (int i = 0; i < EXTENSION_LAST; i++)
    {
        if (strcmp(fileinfo.extension, KNOWN_EXTENSIONS[i]) == 0)
        {
            fileinfo.extension = KNOWN_EXTENSIONS[i];
            fileinfo.extension_index = i;
            break;
        }
    }

    size_t linesize = 0;
    while ((linesize = getlineV2(&LINE_PTR, &LINE_PTR_SIZE, file)) != (size_t)EOF)
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
    argument_parser_parse(&parser, argc, argv);

    if (argument_parser_has(&parser, "-h"))
    {
        print_help_message();
        argument_parser_dispose(&parser);
        return 0;
    }

    PRINT_VERSION;
    logEnableDebugMsgs(1);
    logEnableStdout(1);

    LINE_PTR = malloc(0x1000);
    LINE_PTR_SIZE = 0x1000;

    file_info_t directory_information[EXTENSION_LAST+1];
    file_info_t total_directory_information;
    file_info_t grand_total;
    memset(&grand_total, 0, sizeof(file_info_t));
    for (size_t i = 0; i < parser.unparsed.size; i++)
    {
        memset(directory_information, 0, sizeof(directory_information));
        memset(&total_directory_information, 0, sizeof(file_info_t));

        const char* filePath = parser.unparsed.data[i].values[0];
        if (filePath[strlen(filePath)-1] == '/' || filePath[strlen(filePath)-1] == '\\')
            ((char*)filePath)[strlen(filePath)-1] = 0;

        string_list_t files = file_util_get_directory_contents(filePath, FilterMaskAllFiles);
        for (size_t fidx = 0; fidx < files.count; fidx++)
        {
            file_info_t info = get_file_info(stringf("%s/%s", filePath, files.strings[fidx]));
            if (info.extension_index == -1)
                info.extension_index = EXTENSION_LAST;

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
        string_list_dispose(&files);

        if (total_directory_information.file_count == 0)
            continue;

        printf("Info for '%s'\n===========", filePath);
        for (size_t i = 0; i < strlen(filePath); i++)
            printf("=");
        printf("\n");
        printf("\ttotal:\t%d files | lines: %ld code | %ld empty | (%ld total)\n", total_directory_information.file_count, total_directory_information.non_empty_lines, total_directory_information.empty_lines, total_directory_information.non_empty_lines + total_directory_information.empty_lines);
        for (int i = 0; i < EXTENSION_LAST; i++)
        {
            file_info_t info = directory_information[i];
            if (info.file_count == 0)
                continue;
            printf("\t'%s':\t%d files | lines: %ld code | %ld empty | (%ld total)\n", info.extension, info.file_count, info.non_empty_lines, info.empty_lines, info.non_empty_lines + info.empty_lines);
        }
        file_info_t info = directory_information[EXTENSION_LAST];
        if (info.file_count > 0)
            printf("\tmisc:\t%d files | lines: %ld code | %ld empty | (%ld total)\n", info.file_count, info.non_empty_lines, info.empty_lines, info.non_empty_lines + info.empty_lines);
        printf("\n");
    }

    if (parser.unparsed.size > 1)
    {
        printf("Info Summary\n============\n");
        printf("\ttotal:\t%d files | lines: %ld code | %ld empty | (%ld total)\n\n", grand_total.file_count, grand_total.non_empty_lines, grand_total.empty_lines, grand_total.non_empty_lines + grand_total.empty_lines);
    }

    free(LINE_PTR);

    argument_parser_dispose(&parser);

    return 0;
}
