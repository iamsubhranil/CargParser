#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef struct ArgumentArray* ArgumentList;

ArgumentList arg_list_create(size_t size);

void arg_add(ArgumentList list, const char shorthand, const char *full, bool value_required);

void arg_parse(int argc, char **argv, ArgumentList list);

bool arg_is_present(ArgumentList list, const char shorthand);
char* arg_value(ArgumentList list, const char shorthand);

void arg_free(ArgumentList list);
