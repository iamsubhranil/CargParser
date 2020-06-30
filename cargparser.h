#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct ArgumentArray* ArgumentList;

// creates a new argument list
ArgumentList arg_list_create();

// adds an argument
// list: the argument list
// shorthand: character for '-o' version of the argument
// full: string for '--option' version of the argument
// value_required: whether or not this argument requires a value
// is_optional: whether or not this argument is optional
//              if the argument is non optional, an error will
//              be printed when this is unspecified
void arg_add(ArgumentList list, const char shorthand, const char* full,
             bool value_required, bool is_optional);

// parses the given program arguments
void arg_parse(int argc, char** argv, ArgumentList list);

// returns true if any non optional arguments are unspecified
bool arg_missing_mandatory(ArgumentList list);

// checks whether an argument is present, and returns value
bool arg_is_present(ArgumentList list, const char shorthand);
char* arg_value(ArgumentList list, const char shorthand);

// frees the argument list
void arg_free(ArgumentList list);
