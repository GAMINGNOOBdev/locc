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

#define PRINT_VERSION printf("LOCC version %d.%d.%d\n", __YEAR__, __MONTH__, __DAY__)

void print_help_message(void)
{
    printf("Usage:\n\tlocc [options] <project paths...>\n\tOptions:\n\t\t-h\tPrint this help message\n");
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

    file_info_t directory_information[EXTENSION_LAST+1];
    for (size_t i = 0; i < parser.unparsed.size; i++)
    {
        memset(directory_information, 0, sizeof(directory_information));
        const char* filePath = parser.unparsed.data[i].values[0];
        if (filePath[strlen(filePath)-1] == '/' || filePath[strlen(filePath)-1] == '\\')
            ((char*)filePath)[strlen(filePath)-1] = 0;
        string_list_t files = file_util_get_directory_contents(filePath, FilterMaskAllFiles);
        for (size_t fidx = 0; fidx < files.count; fidx++)
        {
            file_info_t info = file_util_get_file_info(stringf("%s/%s", filePath, files.strings[fidx]));
            if (info.extension_index == -1)
                info.extension_index = EXTENSION_LAST;

            directory_information[info.extension_index].extension = info.extension;
            directory_information[info.extension_index].empty_lines += info.empty_lines;
            directory_information[info.extension_index].non_empty_lines += info.non_empty_lines;
            directory_information[info.extension_index].file_count++;
        }
        string_list_dispose(&files);

        printf("Info for '%s'\n===========", filePath);
        for (size_t i = 0; i < strlen(filePath); i++)
            printf("=");
        printf("\n");
        for (int i = 0; i < EXTENSION_LAST; i++)
        {
            file_info_t info = directory_information[i];
            if (info.file_count == 0)
                continue;
            printf("\t'%s': %d files, %ld lines of code, %ld empty lines (%ld total lines)\n", info.extension, info.file_count, info.non_empty_lines, info.empty_lines, info.non_empty_lines + info.empty_lines);
        }
        file_info_t info = directory_information[EXTENSION_LAST];
        if (info.file_count > 0)
            printf("\tmiscellaneous: %d files, %ld lines of code, %ld empty lines (%ld total lines)\n", info.file_count, info.non_empty_lines, info.empty_lines, info.non_empty_lines + info.empty_lines);
        printf("\n");
    }

    argument_parser_dispose(&parser);

    return 0;
}
