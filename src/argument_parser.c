#include <argument_parser.h>
#include <stdlib.h>
#include <memory.h>

argument create_argument(argument_type type, const char* name, const char* secondName, const char* value)
{
    argument arg;
    memset(&arg, 0, sizeof(argument));

    memcpy(&arg.type, &type, sizeof(argument_type));
    memcpy(&arg.name, &name, sizeof(const char*));
    memcpy(&arg.second_name, &secondName, sizeof(const char*));
    memcpy(&arg.values[0], &value, sizeof(const char*));
    arg.value_count++;

    return arg;
}

/////////////////////
/// argument List ///
/////////////////////

argument* get_matching_argument(argument_list* list, const char* name);

void argument_list_init(argument_list* obj)
{
    if (!obj)
        return;

    memset(obj, 0, sizeof(argument_list));
}

void argument_list_clear(argument_list* obj)
{
    if (!obj)
        return;

    if (obj->data == NULL)
        return;

    free(obj->data);
    argument_list_init(obj);
}

void argument_list_dispose(argument_list* obj)
{
    if (!obj)
        return;

    argument_list_clear(obj);
}

void argument_list_add(argument_list* obj, argument arg)
{
    if (!obj)
        return;

    argument* tmparg = get_matching_argument(obj, arg.name);
    if (tmparg != NULL)
    {
        if (tmparg->value_count >= sizeof(tmparg->values) / sizeof(const char*))
            return;
        tmparg->values[tmparg->value_count] = arg.values[0];
        tmparg->value_count++;
        return;
    }

    if (obj->size + 1 >= obj->buffer_size)
    {
        obj->buffer_size += ARGUMENT_LIST_BUFFER_SIZE;
        obj->data = realloc(obj->data, obj->buffer_size * sizeof(argument));
    }

    obj->data[obj->size] = arg;
    obj->size++;
}

///////////////////////
/// argument Parser ///
///////////////////////

void argument_parser_init(argument_parser* obj)
{
    if (!obj)
        return;

    memset(obj, 0, sizeof(argument_parser));

    argument_list_init(&obj->unparsed);
    argument_list_init(&obj->parsed);
    argument_list_init(&obj->lookup);
}

void argument_parser_dispose(argument_parser* obj)
{
    if (!obj) return;

    argument_list_dispose(&obj->unparsed);
    argument_list_dispose(&obj->parsed);
    argument_list_dispose(&obj->lookup);

    memset(obj, 0, sizeof(argument_parser));
}

void argument_parser_add(argument_parser* obj, argument_type type, const char* name, const char* alternateName)
{
    if (!obj || !name)
        return;

    argument_list_add(&obj->lookup, create_argument(type, name, alternateName, ""));
}

argument* get_matching_argument(argument_list* list, const char* name)
{
    if (!list || !list->size || !name)
        return NULL;

    for (size_t i = 0; i < list->size; i++)
    {
        argument arg = list->data[i];
        if (arg.name == NULL)
            continue;
        
        if (strcmp(name, arg.name) == 0)
            return &list->data[i];

        if (arg.second_name == NULL)
            continue;

        if (strcmp(name, arg.second_name) == 0)
            return &list->data[i];
    }

    return NULL;
}

void argument_parser_parse(argument_parser* obj, int argc, const char** argv)
{
    if (!obj) return;

    argument_list_clear(&obj->unparsed);
    argument_list_clear(&obj->parsed);

    for (int i = 1; i < argc; i++)
    {
        const char* argStr = argv[i];
        if (argStr[0] != '-')
        {
            argument_list_add(&obj->unparsed, create_argument(Argument_Type_Invalid, argStr, argStr, argStr));
            continue;
        }

        argument* lookupargument = get_matching_argument(&obj->lookup, argStr);
        if (!lookupargument)
            continue;

        if (lookupargument->type == Argument_Type_Value)
        {
            if (i == argc - 1) break;

            argument_list_add(&obj->parsed, create_argument(Argument_Type_Value, lookupargument->name, lookupargument->second_name, argv[i+1]));
            i++;
            continue;
        }

        argument_list_add(&obj->parsed, create_argument(Argument_Type_Flag, lookupargument->name, lookupargument->second_name, ""));
    }
}

argument* argument_parser_get(argument_parser* obj, const char* name)
{
    if (!obj)
        return NULL;

    return get_matching_argument(&obj->parsed, name);
}

uint8_t argument_parser_has(argument_parser* obj, const char* name)
{
    return argument_parser_get(obj, name) != NULL;
}
