#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct ArgumentArray* ArgumentList;

// creates a new argument list
ArgumentList arg_list_create();

// adds an argument
// list: the argument list
// shorthand: character for '-o' version of the argument
//            this character is used as the identifier for
//            the argument everywhere
// full: string for '--option' version of the argument
// help: string for help message
// value_required: whether or not this argument requires a value
// is_optional: whether or not this argument is optional
//              if the argument is non optional, an error will
//              be printed when this is unspecified
void arg_add(ArgumentList list, const char shorthand, const char* full,
             const char* help, bool value_required, bool is_optional);

// Does the same as before, but adds this argument as a positional
// argument, rather than a flag based argument.
// Positional arguments are parsed in the order of insertion, and
// non optional argument first manner. i.e., consider the following:
//
// arg_add_positional(list, 'i', "input_file", "File to read", true);
// arg_add_positional(list, 'o', "output_file", "File to write", false);
// arg_add_positional(list, 'g', "generator", "Generator to use", true);
// arg_add_positional(list, 'v', "version", "Version to run", false);
//
// Then the arguments will be parsed as:
//  output_file version [input_file] [generator]
//
// The argument should be queried using the shorthand specifier as before.
// Specifing a value is mandatory for positional arguments, because they
// are, by the very nature, _positional_ arguments.
void arg_add_positional(ArgumentList list, const char shorthand,
                        const char* full, const char* help, bool is_optional);

// parses the given program arguments
void arg_parse(int argc, char** argv, ArgumentList list);

// returns true if any non optional arguments are unspecified
bool arg_missing_mandatory(ArgumentList list);

// checks whether an argument is present, and returns value
bool arg_is_present(ArgumentList list, const char shorthand);
char* arg_value(ArgumentList list, const char shorthand);

// print an usage of the program with the given argument list
void arg_print_default_help(ArgumentList list, char** argv);

// frees the argument list
void arg_free(ArgumentList list);
